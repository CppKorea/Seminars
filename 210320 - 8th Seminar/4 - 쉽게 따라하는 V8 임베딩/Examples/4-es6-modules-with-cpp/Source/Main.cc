// Copyright (c) 2021 Chanjung Kim. All rights reserved.
// Licensed under the MIT License.

#include <libplatform/libplatform.h>
#include <src/debug/debug-interface.h>
#include <v8.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

namespace fs = std::filesystem;

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

void ReadStringFromFile(v8::FunctionCallbackInfo<v8::Value> const& info)
{
    v8::Isolate* isolate = info.GetIsolate();
    if (info.Length() != 1)
    {
        v8::Local<v8::String> msg
            = v8::String::NewFromUtf8Literal(isolate, "Invalid number of arguments!");
        isolate->ThrowException(v8::Exception::Error(msg));
        return;
    }

    v8::Local<v8::Value> filePathValue = info[0];
    if (!filePathValue->IsString() && !filePathValue->IsStringObject())
    {
        v8::Local<v8::String> msg
            = v8::String::NewFromUtf8Literal(isolate, "Given path is not a string");
        isolate->ThrowException(v8::Exception::Error(msg));
        return;
    }
    v8::String::Utf8Value filePathUtf8Value { isolate, filePathValue };

    if (fs::is_directory(*filePathUtf8Value))
    {
        v8::Local<v8::String> msg
            = v8::String::NewFromUtf8Literal(isolate, "Given path is a directory");
        isolate->ThrowException(v8::Exception::Error(msg));
        return;
    }

    std::ifstream ifs { *filePathUtf8Value };
    if (!ifs)
    {
        v8::Local<v8::String> msg = v8::String::NewFromUtf8Literal(isolate, "No such file exists");
        isolate->ThrowException(v8::Exception::Error(msg));
        return;
    }

    ifs.seekg(0, std::ios::end);
    size_t size = ifs.tellg();

    std::string content(size, ' ');
    ifs.seekg(0);
    ifs.read(std::addressof(content[0]), size);

    v8::MaybeLocal<v8::String> maybeContentValue = v8::String::NewFromUtf8(isolate, content.data());
    if (maybeContentValue.IsEmpty())
    {
        v8::Local<v8::String> msg
            = v8::String::NewFromUtf8Literal(isolate, "Given file contains invalid character");
        isolate->ThrowException(v8::Exception::Error(msg));
        return;
    }

    info.GetReturnValue().Set(maybeContentValue.ToLocalChecked());
}

std::map<std::string, v8::Local<v8::Module>> modules;

v8::MaybeLocal<v8::Value> SyntheticModuleCallback(v8::Local<v8::Context> context,
                                                  v8::Local<v8::Module>  module)
{
    v8::Isolate*             isolate = context->GetIsolate();
    v8::EscapableHandleScope handleScope { isolate };

    {
        v8::Local<v8::Function> readStringFromFile
            = v8::Function::New(context, ReadStringFromFile).ToLocalChecked();
        v8::Local<v8::String> exportName
            = v8::String::NewFromUtf8(isolate, "readStringFromFile").ToLocalChecked();
        module->SetSyntheticModuleExport(exportName, readStringFromFile);
    }

    return handleScope.Escape(v8::Boolean::New(isolate, true));
}

v8::MaybeLocal<v8::Module> ModuleResolveCallback(v8::Local<v8::Context> context,
                                                 v8::Local<v8::String>  specifier,
                                                 v8::Local<v8::Module>  referrer)
{
    v8::Isolate*          isolate = context->GetIsolate();
    v8::String::Utf8Value specifierName { isolate, specifier };

    if (auto it = modules.find(*specifierName); it != modules.end()) return it->second;

    v8::Local<v8::String> msg = v8::String::NewFromUtf8Literal(isolate, "No such module exists");
    isolate->ThrowException(v8::Exception::Error(msg));
    return v8::MaybeLocal<v8::Module> {};
}

void Repl(v8::Isolate* isolate)
{
    v8::Isolate::Scope isolateScope { isolate };
    v8::HandleScope    handleScope { isolate };

    v8::Local<v8::Context> context = v8::Context::New(isolate, nullptr);
    v8::Context::Scope     contextScope { context };

    v8::Local<v8::String> fileUtilsModuleName
        = v8::String::NewFromUtf8(isolate, u8"fileUtils").ToLocalChecked();
    std::vector<v8::Local<v8::String>> fileUtilsExportNames {
        v8::String::NewFromUtf8(isolate, "readStringFromFile").ToLocalChecked(),
    };
    v8::Local<v8::Module> fileUtilsModule = v8::Module::CreateSyntheticModule(
        isolate, fileUtilsModuleName, fileUtilsExportNames, SyntheticModuleCallback);
    modules.insert(std::make_pair("fileUtils", fileUtilsModule));

    ConsoleDelegate delegate { isolate };
    v8::debug::SetConsoleDelegate(isolate, &delegate);

    std::string code, line;
    for (std::cout << "> "; std::getline(std::cin, line); std::cout << "> ")
    {
        if (!line.empty() && line[0] == '.')
        {
            auto it = std::find(line.begin(), line.end(), ' ');

            if (line.compare(0, it - line.begin(), ".exit") == 0) break;
            else if (line.compare(0, it - line.begin(), ".module") == 0)
            {
                std::string originName(it + 1, line.end());
                if (originName.empty())
                {
                    std::cout << "(REPL error) Module name is empty" << std::endl;
                    continue;
                }

                v8::TryCatch          tryCatch { isolate };
                v8::Local<v8::String> source
                    = v8::String::NewFromUtf8(isolate, code.c_str()).ToLocalChecked();
                code.clear();

                v8::ScriptCompiler::Source script {
                    source,
                    v8::ScriptOrigin {
                        v8::String::NewFromUtf8(isolate, originName.c_str()).ToLocalChecked(),
                        v8::Integer::New(isolate, 0),
                        v8::Integer::New(isolate, 0),
                        v8::False(isolate),
                        v8::Integer::New(isolate, 0),
                        v8::Null(isolate),
                        v8::False(isolate),
                        v8::False(isolate),
                        v8::True(isolate),
                    },
                };

                v8::MaybeLocal<v8::Module> maybeModule
                    = v8::ScriptCompiler::CompileModule(isolate, &script);
                if (maybeModule.IsEmpty())
                {
                    std::cout << "(Compile error) ";
                    v8::Local<v8::Value>  exception = tryCatch.Exception();
                    v8::String::Utf8Value utf8(isolate, exception);
                    std::cout << *utf8 << std::endl;
                    continue;
                }
                v8::Local<v8::Module> module = maybeModule.ToLocalChecked();

                v8::Maybe<bool> moduleInit
                    = module->InstantiateModule(context, ModuleResolveCallback);
                if (moduleInit.IsNothing() || !moduleInit.ToChecked())
                {
                    std::cout << "(Instantiation error) ";
                    v8::Local<v8::Value>  exception = tryCatch.Exception();
                    v8::String::Utf8Value utf8(isolate, exception);
                    std::cout << *utf8 << std::endl;
                    continue;
                }

                v8::MaybeLocal<v8::Value> maybeResult = module->Evaluate(context);
                if (maybeResult.IsEmpty())
                {
                    std::cout << "(Runtime error) ";
                    v8::Local<v8::Value>  exception = tryCatch.Exception();
                    v8::String::Utf8Value utf8(isolate, exception);
                    std::cout << *utf8 << std::endl;
                    continue;
                }
                v8::Local<v8::Value> result = maybeResult.ToLocalChecked();

                modules.insert(std::make_pair(originName, module));

                if (!result->IsUndefined())
                {
                    v8::String::Utf8Value utf8(isolate, result);
                    std::cout << *utf8 << std::endl;
                }
            }
        }
        else if (line.empty())
        {
            v8::Local<v8::String> source
                = v8::String::NewFromUtf8(isolate, code.c_str()).ToLocalChecked();
            code.clear();

            v8::TryCatch tryCatch { isolate };

            v8::MaybeLocal<v8::Script> maybeScript = v8::Script::Compile(context, source);
            if (maybeScript.IsEmpty())
            {
                std::cout << "(Compile error) ";
                v8::Local<v8::Value>  exception = tryCatch.Exception();
                v8::String::Utf8Value utf8(isolate, exception);
                std::cout << *utf8 << std::endl;
                continue;
            }
            v8::Local<v8::Script> script = maybeScript.ToLocalChecked();

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