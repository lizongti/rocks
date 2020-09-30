#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include "core.h"

using namespace v8;
using namespace node;

namespace stream_packet {

void get(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  args.GetReturnValue().Set(Number::New(isolate, core::get()));
};

void put(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 1) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong number of arguements")));
    return;
  }
  if (!args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Argument 0 is not a number")));
    return;
  }
  uint64_t index = Local<Number>::Cast(args[0])
                       ->IntegerValue(isolate->GetCurrentContext())
                       .FromMaybe(0);
  core::put(index);
};

void size(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 1) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong number of arguements")));
    return;
  }
  if (!args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Argument 0 is not a number")));
    return;
  }
  uint64_t index = Local<Number>::Cast(args[0])
                       ->IntegerValue(isolate->GetCurrentContext())
                       .FromMaybe(0);
  args.GetReturnValue().Set(Number::New(isolate, core::size(index)));
}

void recv(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 2) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong number of arguements")));
    return;
  }
  if (!args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Argument 0 is not a number")));
    return;
  }
  if (!args[1]->IsString()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Argument 1 is not a string")));
    return;
  }
  if (args.Length() >= 3 && !args[2]->IsBoolean()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Argument 2 is not a boolean")));
    return;
  }

  uint64_t index = Local<Number>::Cast(args[0])
                       ->IntegerValue(isolate->GetCurrentContext())
                       .FromMaybe(0);
  Local<String> str = Local<String>::Cast(args[1]);
  int32_t len = str->Utf8Length(isolate);
  char buf[len];
  str->WriteUtf8(isolate, buf, len);
  std::string flow(buf, len);
  bool validate = false;
  if (args.Length() >= 3) {
    validate = Local<Boolean>::Cast(args[2])
                   ->BooleanValue(isolate->GetCurrentContext())
                   .FromMaybe(false);
  }

  core::message msg;
  int32_t state = core::recv(index, flow, msg, validate);

  Local<Object> object = Object::New(isolate);
  object->Set(String::NewFromUtf8(isolate, "state"),
              Number::New(isolate, state));
  if (state > 0) {
    object->Set(String::NewFromUtf8(isolate, "id"),
                Number::New(isolate, msg.id));

    object->Set(String::NewFromUtf8(isolate, "msg"),
                String::NewFromUtf8(isolate, msg.data.c_str(),
                                    NewStringType::kNormal, msg.data.length())
                    .ToLocalChecked());
  }
  args.GetReturnValue().Set(object);
};

void send(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 3) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong number of arguements")));
    return;
  }
  if (!args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Argument 0 is not a number")));
    return;
  }
  if (!args[1]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Argument 0 is not a number")));
    return;
  }
  if (!args[2]->IsString()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Argument 1 is not a string")));
    return;
  }
  if (args.Length() >= 4 && !args[3]->IsBoolean()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Argument 3 is not a boolean")));
    return;
  }
  uint64_t index = Local<Number>::Cast(args[0])
                       ->IntegerValue(isolate->GetCurrentContext())
                       .FromMaybe(0);
  int32_t id = Local<Number>::Cast(args[1])
                   ->Int32Value(isolate->GetCurrentContext())
                   .FromMaybe(0);
  Local<String> str = Local<String>::Cast(args[2]);
  int32_t len = str->Utf8Length(isolate);
  char buf[len];
  str->WriteUtf8(isolate, buf, len);
  std::string data(buf, len);
  bool validate = false;
  if (args.Length() >= 4) {
    validate = Local<Boolean>::Cast(args[3])
                   ->BooleanValue(isolate->GetCurrentContext())
                   .FromMaybe(false);
  }

  core::message msg({id, data});
  std::string flow;
  int32_t state = core::send(index, msg, flow, validate);

  Local<Object> object = Object::New(isolate);
  object->Set(String::NewFromUtf8(isolate, "state"),
              Number::New(isolate, state));
  if (state > 0) {
    object->Set(String::NewFromUtf8(isolate, "data"),
                String::NewFromUtf8(isolate, flow.c_str(),
                                    NewStringType::kNormal, flow.length())
                    .ToLocalChecked());
  }
  args.GetReturnValue().Set(object);
}

void Initialize(Local<Object> exports) {
  NODE_SET_METHOD(exports, "get", get);
  NODE_SET_METHOD(exports, "put", put);
  NODE_SET_METHOD(exports, "recv", recv);
  NODE_SET_METHOD(exports, "send", send);
  NODE_SET_METHOD(exports, "size", size);
};

NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)

};  // namespace stream_packet
