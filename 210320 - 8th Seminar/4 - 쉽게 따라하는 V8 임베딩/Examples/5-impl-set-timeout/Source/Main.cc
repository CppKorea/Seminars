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

struct TimeoutQueueItem
{
    chrono::time_point<chrono::steady_clock> firesAt;
    v8::Global<v8::Function>                 functionToCall;
    std::vector<v8::Global<v8::Value>>       arguments;
};

using TimeoutQueueItemPtr = std::shared_ptr<TimeoutQueueItem>;

struct TimeoutQueueItemComparator
{
    constexpr bool operator()(TimeoutQueueItemPtr const& lhs, TimeoutQueueItemPtr const& rhs) const
    {
        return lhs->firesAt > rhs->firesAt;
    }
};

// clang-format off
std::priority_queue<TimeoutQueueItemPtr,
                    std::vector<TimeoutQueueItemPtr>,
                    TimeoutQueueItemComparator> timeoutQueue;
// clang-format on

void SetTimeout(v8::FunctionCallbackInfo<v8::Value> const& info)
{
    v8::Isolate* isolate = info.GetIsolate();
    if (info.Length() < 2)
    {
        v8::Local<v8::String> msg
            = v8::String::NewFromUtf8Literal(isolate, "Invalid number of arguments!");
        isolate->ThrowException(v8::Exception::Error(msg));
        return;
    }

    v8::Local<v8::Value> functionValue = info[0];
    if (!functionValue->IsFunction())
    {
        v8::Local<v8::String> msg = v8::String::NewFromUtf8Literal(isolate, "Not a function");
        isolate->ThrowException(v8::Exception::Error(msg));
        return;
    }
    v8::Local<v8::Function> function = functionValue.As<v8::Function>();

    v8::Local<v8::Value> milliValue = info[1];
    if (!milliValue->IsNumber() && !milliValue->IsNumberObject())
    {
        v8::Local<v8::String> msg = v8::String::NewFromUtf8Literal(isolate, "Not a number");
        isolate->ThrowException(v8::Exception::Error(msg));
        return;
    }
    double milli      = milliValue->NumberValue(isolate->GetCurrentContext()).ToChecked();
    auto   duration   = chrono::duration<double, std::milli> { milli };
    auto   willFireAt = chrono::steady_clock::now() + duration;

    std::vector<v8::Global<v8::Value>> arguments;
    for (int i = 2; i < info.Length(); ++i)
        arguments.push_back(v8::Global<v8::Value> { isolate, info[i] });

    timeoutQueue.push(TimeoutQueueItemPtr {
        new TimeoutQueueItem {
            chrono::time_point_cast<chrono::milliseconds>(willFireAt),
            v8::Global<v8::Function> { isolate, function },
            std::move(arguments),
        },
    });
}

void Repl(v8::Isolate* isolate)
{
    v8::Isolate::Scope isolateScope { isolate };
    v8::HandleScope    handleScope { isolate };

    v8::Local<v8::FunctionTemplate> setTimeout = v8::FunctionTemplate::New(isolate, SetTimeout);
    v8::Local<v8::ObjectTemplate>   global     = v8::ObjectTemplate::New(isolate);
    global->Set(isolate, "setTimeout", setTimeout);

    v8::Local<v8::Context> context = v8::Context::New(isolate, nullptr, global);
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
                std::cout << *utf8 << std::endl;
            }

            while (!timeoutQueue.empty())
            {
                auto item = std::move(*timeoutQueue.top());
                timeoutQueue.pop();

                if (item.firesAt >= chrono::steady_clock::now())
                    std::this_thread::sleep_until(item.firesAt);

                std::vector<v8::Local<v8::Value>> arguments;
                for (auto& argument : item.arguments) arguments.push_back(argument.Get(isolate));

                item.functionToCall.Get(isolate)->CallAsFunction(
                    context, context->Global(), (int)arguments.size(), arguments.data());
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