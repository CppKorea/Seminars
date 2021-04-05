// Copyright (c) 2021 Chanjung Kim. All rights reserved.
// Licensed under the MIT License.

#include <libplatform/libplatform.h>
#include <src/debug/debug-interface.h>
#include <v8.h>

#include <filesystem>
#include <fstream>
#include <iostream>
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

class Vec
{
  public:
    constexpr static uint16_t Tag = 1623;

    // https://itnext.io/v8-wrapped-objects-lifecycle-42272de712e0
    class HandleVisitor : public v8::PersistentHandleVisitor
    {
      private:
        v8::Isolate* _isolate;

      public:
        HandleVisitor(v8::Isolate* isolate) : _isolate { isolate } {}

      public:
        virtual void VisitPersistentHandle(v8::Persistent<v8::Value>* value,
                                           uint16_t                   classId) override
        {
            v8::Local<v8::Context> current = _isolate->GetCurrentContext();
            if (classId == Vec::Tag)
            {
                auto local = value->Get(_isolate);
                if (local->IsObject())
                {
                    void* ptr = local.As<v8::Object>()->GetAlignedPointerFromInternalField(0);
                    delete static_cast<Vec*>(ptr);
                }
            }
        }
    };

  private:
    double                 _x, _y;
    v8::Global<v8::Object> _global;

  public:
    Vec(v8::Isolate* isolate, v8::Local<v8::Object> local, double x, double y) :
        _x { x },
        _y { y },
        _global { isolate, local }
    {
        _global.SetWeak(this, WeakHandler, v8::WeakCallbackType::kParameter);
        _global.SetWrapperClassId(Vec::Tag);
        std::cout << "From C++: Vec(" << x << ", " << y << ")" << std::endl;
    }

    ~Vec()
    {
        std::cout << "From C++: ~Vec(" << _x << ", " << _y << ")" << std::endl;
    }

  private:
    static void WeakHandler(v8::WeakCallbackInfo<Vec> const& data)
    {
        delete data.GetParameter();
    }

    static void Constructor(v8::FunctionCallbackInfo<v8::Value> const& info)
    {
        v8::Isolate*           isolate = info.GetIsolate();
        v8::Local<v8::Context> context = isolate->GetCurrentContext();
        if (!info.IsConstructCall())
        {
            v8::Local<v8::String> msg
                = v8::String::NewFromUtf8Literal(isolate, "Vec must be called as a constructor");
            isolate->ThrowException(v8::Exception::Error(msg));
            return;
        }

        if (info.Length() != 2)
        {
            v8::Local<v8::String> msg
                = v8::String::NewFromUtf8Literal(isolate, "Invalid number of arguments");
            isolate->ThrowException(v8::Exception::Error(msg));
            return;
        }

        v8::Local<v8::Value> value0 = info[0];
        if (!value0->IsNumber() && !value0->IsNumberObject())
        {
            v8::Local<v8::String> msg
                = v8::String::NewFromUtf8Literal(isolate, "x is not a number");
            isolate->ThrowException(v8::Exception::Error(msg));
            return;
        }
        double number0 = value0->NumberValue(context).ToChecked();

        v8::Local<v8::Value> value1 = info[1];
        if (!value0->IsNumber() && !value0->IsNumberObject())
        {
            v8::Local<v8::String> msg
                = v8::String::NewFromUtf8Literal(isolate, "y is not a number");
            isolate->ThrowException(v8::Exception::Error(msg));
            return;
        }
        double number1 = value1->NumberValue(context).ToChecked();

        info.This()->SetAlignedPointerInInternalField(
            0, new Vec { isolate, info.This(), number0, number1 });
    }

    static void GetX(v8::Local<v8::String>                      property,
                     const v8::PropertyCallbackInfo<v8::Value>& info)
    {
        v8::Local<v8::Object> self = info.This();

        void*  ptr   = self->GetAlignedPointerFromInternalField(0);
        double value = static_cast<Vec*>(ptr)->_x;
        info.GetReturnValue().Set(value);
    }

    static void SetX(v8::Local<v8::String>                 property,
                     v8::Local<v8::Value>                  value,
                     const v8::PropertyCallbackInfo<void>& info)
    {
        v8::Isolate*           isolate = info.GetIsolate();
        v8::Local<v8::Context> context = isolate->GetCurrentContext();
        v8::Local<v8::Object>  self    = info.This();

        void* ptr                  = self->GetAlignedPointerFromInternalField(0);
        static_cast<Vec*>(ptr)->_x = value->NumberValue(context).ToChecked();
    }

    static void GetY(v8::Local<v8::String>                      property,
                     const v8::PropertyCallbackInfo<v8::Value>& info)
    {
        v8::Local<v8::Object> self = info.This();

        void*  ptr   = self->GetAlignedPointerFromInternalField(0);
        double value = static_cast<Vec*>(ptr)->_y;
        info.GetReturnValue().Set(value);
    }

    static void SetY(v8::Local<v8::String>                 property,
                     v8::Local<v8::Value>                  value,
                     const v8::PropertyCallbackInfo<void>& info)
    {
        v8::Isolate*           isolate = info.GetIsolate();
        v8::Local<v8::Context> context = isolate->GetCurrentContext();
        v8::Local<v8::Object>  self    = info.This();

        void* ptr                  = self->GetAlignedPointerFromInternalField(0);
        static_cast<Vec*>(ptr)->_y = value->NumberValue(context).ToChecked();
    }

    static void Norm(v8::FunctionCallbackInfo<v8::Value> const& info)
    {
        v8::Local<v8::Object> self = info.This();

        void* ptr = self->GetAlignedPointerFromInternalField(0);
        Vec*  vec = static_cast<Vec*>(ptr);

        double rtn = sqrt(vec->_x * vec->_x + vec->_y * vec->_y);
        info.GetReturnValue().Set(rtn);
    }

  public:
    static v8::Local<v8::FunctionTemplate> MakeConstructor(v8::Isolate* isolate)
    {
        v8::Local<v8::FunctionTemplate> constructor
            = v8::FunctionTemplate::New(isolate, Constructor);

        v8::Local<v8::ObjectTemplate> prototype = constructor->PrototypeTemplate();

        v8::Local<v8::String> xName = v8::String::NewFromUtf8(isolate, "x").ToLocalChecked();
        prototype->SetAccessor(xName, GetX, SetX);

        v8::Local<v8::String> yName = v8::String::NewFromUtf8(isolate, "y").ToLocalChecked();
        prototype->SetAccessor(yName, GetY, SetY);

        prototype->Set(isolate, "norm", v8::FunctionTemplate::New(isolate, Norm));

        v8::Local<v8::ObjectTemplate> instance = constructor->InstanceTemplate();
        instance->SetInternalFieldCount(1);

        return constructor;
    }
};

void Repl(v8::Isolate* isolate)
{
    std::ios_base::sync_with_stdio(false);

    v8::Isolate::Scope isolateScope { isolate };
    v8::HandleScope    handleScope { isolate };

    v8::Local<v8::FunctionTemplate> vec    = Vec::MakeConstructor(isolate);
    v8::Local<v8::ObjectTemplate>   global = v8::ObjectTemplate::New(isolate);
    global->Set(isolate, "Vec", vec);

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
        }
        else
        {
            code.push_back('\n');
            code.append(line);
        }
    }

    Vec::HandleVisitor visitor { isolate };
    isolate->VisitHandlesWithClassIds(&visitor);
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