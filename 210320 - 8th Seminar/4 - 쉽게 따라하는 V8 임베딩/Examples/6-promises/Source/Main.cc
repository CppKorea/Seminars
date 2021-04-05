// Copyright (c) 2021 Chanjung Kim. All rights reserved.
// Licensed under the MIT License.

#include <libplatform/libplatform.h>
#include <src/debug/debug-interface.h>
#include <v8.h>

#include <chrono>
#include <iostream>
#include <queue>
#include <string>
#include <thread>

namespace chrono = std::chrono;

class ConsoleDelegate : public v8::debug::ConsoleDelegate
{
  private:
    v8::Isolate* _isolate;

  public:
    ConsoleDelegate(v8::Isolate* isolate) : _isolate { isolate } {}

  public:
    virtual void Log(const v8::debug::ConsoleCallArguments& args,
                     const v8::debug::ConsoleContext&       context)
    {
        for (int i = 0; i < args.Length(); ++i)
        {
            if (i != 0) std::cout << ", ";
            v8::String::Utf8Value utf8(_isolate, args[i]);
            std::cout << *utf8;
        }
        std::cout << std::endl;
    }
};

struct SleepQueueItem
{
    chrono::time_point<chrono::steady_clock> firesAt;
    v8::Global<v8::Promise::Resolver>        resolver;
};

using SleepQueueItemPtr = std::shared_ptr<SleepQueueItem>;

struct SleepQueueItemComparator
{
    constexpr bool operator()(SleepQueueItemPtr const& lhs, SleepQueueItemPtr const& rhs) const
    {
        return lhs->firesAt > rhs->firesAt;
    }
};

// clang-format off
std::priority_queue<SleepQueueItemPtr,
                    std::vector<SleepQueueItemPtr>,
                    SleepQueueItemComparator> sleepQueue;
// clang-format on

void Sleep(v8::FunctionCallbackInfo<v8::Value> const& info)
{
    v8::Isolate*           isolate = info.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    if (info.Length() != 1)
    {
        v8::Local<v8::String> msg
            = v8::String::NewFromUtf8Literal(isolate, "Invalid number of arguments!");
        isolate->ThrowException(v8::Exception::Error(msg));
        return;
    }

    v8::Local<v8::Value> milliValue = info[0];
    if (!milliValue->IsNumber() && !milliValue->IsNumberObject())
    {
        v8::Local<v8::String> msg = v8::String::NewFromUtf8Literal(isolate, "Not a number");
        isolate->ThrowException(v8::Exception::Error(msg));
        return;
    }
    double milli      = milliValue->NumberValue(isolate->GetCurrentContext()).ToChecked();
    auto   duration   = chrono::duration<double, std::milli> { milli };
    auto   willFireAt = chrono::steady_clock::now() + duration;

    v8::Local<v8::Promise::Resolver> resolver
        = v8::Promise::Resolver::New(context).ToLocalChecked();

    sleepQueue.push(SleepQueueItemPtr {
        new SleepQueueItem {
            chrono::time_point_cast<chrono::milliseconds>(willFireAt),
            v8::Global<v8::Promise::Resolver> { isolate, resolver },
        },
    });

    info.GetReturnValue().Set(resolver->GetPromise());
}
void Repl(v8::Isolate* isolate)
{
    v8::Isolate::Scope isolateScope { isolate };
    v8::HandleScope    handleScope { isolate };

    v8::Local<v8::FunctionTemplate> sleep  = v8::FunctionTemplate::New(isolate, Sleep);
    v8::Local<v8::ObjectTemplate>   global = v8::ObjectTemplate::New(isolate);
    global->Set(isolate, "sleep", sleep);

    std::unique_ptr<v8::MicrotaskQueue> microtaskQueue
        = v8::MicrotaskQueue::New(isolate, v8::MicrotasksPolicy::kExplicit);

    v8::Local<v8::Context> context = v8::Context::New(isolate,
                                                      nullptr,
                                                      global,
                                                      v8::MaybeLocal<v8::Value> {},
                                                      v8::DeserializeInternalFieldsCallback {},
                                                      microtaskQueue.get());
    v8::Context::Scope     contextScope { context };

    ConsoleDelegate delegate { isolate };
    v8::debug::SetConsoleDelegate(isolate, &delegate);

    std::string code, line;
    for (std::cout << "> "; std::getline(std::cin, line); std::cout << "> ")
    {
        if (line == ".exit") break;
        else if (line.empty())
        {
            v8::Local<v8::String> source
                = v8::String::NewFromUtf8(isolate, code.c_str()).ToLocalChecked();
            code.clear();

            v8::TryCatch               tryCatch { isolate };
            v8::MaybeLocal<v8::Script> maybeScript = v8::Script::Compile(context, source);

            if (maybeScript.IsEmpty())
            {
                std::cout << "(Compile error) ";
                v8::Local<v8::Value>  exception = tryCatch.Exception();
                v8::String::Utf8Value utf8(isolate, exception);
                std::cout << *utf8 << std::endl;
                continue;
            }

            v8::Local<v8::Script>     script      = maybeScript.ToLocalChecked();
            v8::MaybeLocal<v8::Value> maybeResult = script->Run(context);

            if (maybeResult.IsEmpty())
            {
                std::cout << "(Runtime error) ";
                v8::Local<v8::Value>  exception = tryCatch.Exception();
                v8::String::Utf8Value utf8(isolate, exception);
                std::cout << *utf8 << std::endl;
                continue;
            }

            v8::Local<v8::Value> result = maybeResult.ToLocalChecked();

            if (!result->IsUndefined())
            {
                v8::String::Utf8Value utf8(isolate, result);
                std::cout << "Execution complete: " << *utf8 << std::endl;
            }
            else
            {
                std::cout << "Execution complete" << std::endl;
            }

            microtaskQueue->PerformCheckpoint(isolate);

            while (!sleepQueue.empty())
            {
                auto item = std::move(*sleepQueue.top());
                sleepQueue.pop();

                if (item.firesAt >= chrono::steady_clock::now())
                    std::this_thread::sleep_until(item.firesAt);
                item.resolver.Get(isolate)->Resolve(context, v8::Undefined(isolate)).ToChecked();

                std::cout << "sleep resolved" << std::endl;

                microtaskQueue->PerformCheckpoint(isolate);
            }
        }
        else
        {
            code.push_back('\n');
            code.append(line);
        }
    }
}

int main()
{
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate                 = v8::Isolate::New(create_params);

    Repl(isolate);

    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete create_params.array_buffer_allocator;

    return 0;
}