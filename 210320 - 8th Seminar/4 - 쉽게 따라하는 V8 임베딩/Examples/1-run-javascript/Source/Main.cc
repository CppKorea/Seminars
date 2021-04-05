// Copyright (c) 2021 Chanjung Kim. All rights reserved.
// Licensed under the MIT License.

#include <libplatform/libplatform.h>
#include <src/debug/debug-interface.h>
#include <v8.h>

#include <iostream>
#include <string>

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

void Repl(v8::Isolate* isolate)
{
    v8::Isolate::Scope isolateScope { isolate };
    v8::HandleScope    handleScope { isolate };

    v8::Local<v8::Context> context = v8::Context::New(isolate);
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