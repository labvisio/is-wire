
# Wire

Pub/Sub middleware for the *is* architecture (C++ implementation)

## Installation 

The library is available as a [conan package](https://bintray.com/labviros/is/is-wire%3Ais), to use it simply add is-wire/VERSION@is/stable to the dependencies of your conanfile.

## Usage

### Prepare environment

In order to send/receive messages an amqp broker is necessary, to create one simply run:

```shell
docker run -d --rm -p 5672:5672 -p 15672:15672 rabbitmq:3.7.6-management
```

### Basic send/receive

Create a channel to connect to a broker, create a subscription and subscribe to desired topics to receive messages:

```c++
#include "is/wire/core.hpp"

int main() {
  // Connect to the broker
  auto channel = is::Channel{"amqp://guest:guest@localhost:5672"};

  // Subscribe to the desired topic(s)
  auto subscription = is::Subscription{channel};
  subscription.subscribe(/*topic=*/"MyTopic.SubTopic");
  // ... subscription.subscribe("Other.Topic")

  // Blocks forever waiting for one message from any subscription
  auto message = channel.consume();
  is::info("topic={} body={}", message.topic(), message.body());
}
```

### Create and publish messages:

```c++
#include "is/wire/core.hpp"

int main() {
  // Connect to the broker
  auto channel = is::Channel{"amqp://guest:guest@localhost:5672"};

  auto message = Message{};
  // Fill the payload of our message
  message.set_body("Hello!");

  // Broadcast message to anyone interested (subscribed)
  channel.publish(/*topic=*/"MyTopic.SubTopic", message);
}
```

### Serialize/Deserialize protobuf objects:

```c++
#include "is/wire/core.hpp"
#include "google/protobuf/struct.pb.h"

int main() {
  // Connect to the broker
  auto channel = is::Channel{"amqp://guest:guest@localhost:5672"};

  // Subscribe to the desired topic(s)
  auto subscription = is::Subscription{channel};
  subscription.subscribe(/*topic=*/"MyTopic.SubTopic");

  // Fill our payload object
  auto object = google::protobuf::Struct{};
  auto apples = *object.mutable_fields()["apples"];
  apples.set_string_value("red");

  // Create message 
  auto message = Message{};
  message.set_content_type(is::ContentType::JSON); // or is::ContentType::PROTOBUF
  message.pack(object); // Serialize the object into the message body

  // Broadcast message to anyone interested (subscribed)
  channel.publish(/*topic=*/"MyTopic.SubTopic", message);

  // Blocks forever waiting for the message we just sent
  auto received_message = channel.consume();
  
  // Deserialize the object from the message body
  auto received_struct = received_message.unpack<google::protobuf::Struct>();

  // Check that they are equal
  assert(struct == received_struct);
}
```

### Basic Request/Reply 

Create a RPC Server:

```c++
#include <thread>
#include <chrono>
#include "is/wire/core.hpp"
#include "is/wire/rpc.hpp"
#include "google/protobuf/struct.pb.h"

using namespace google::protobuf;

// Our service implementation
is::Status increment(is::Context* ctx, Struct const& request, Struct* reply) {
  auto value = request.fields()["value"].number_value;
  if (value < 0) {
    // Return error to client
    return is::Status{StatusCode.INVALID_ARGUMENT, "Number must be positive"};
  }

  auto result = *reply->mutable_fields()["result"];
  result.set_number_value(value + 1.0);

  std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Simulate work

  return is::make_status(is::wire::StatusCode::OK);
}

int main(int, char**) {
  auto channel = is::Channel{"amqp://localhost"};

  auto provider = is::ServiceProvider{channel};

  auto logs = is::LogInterceptor{};
  provider.add_interceptor(logs);

  provider.delegate<Struct, Struct>("MyService.Increment", increment);
  provider.run();
}
```

Send a request to the RPC Server:

```c++
#include <chrono>
#include "is/wire/core.hpp"
#include "is/wire/rpc.hpp"
#include "google/protobuf/struct.pb.h"

using namespace google::protobuf;

int main(int, char**) {
  auto channel = is::Channel{"amqp://localhost"};
  auto subscription = is::Subscription{channel};

  auto object = google::protobuf::Struct{};
  auto value = *request.mutable_fields()["value"];
  value.set_number_value(15.0);

  auto request = is::Message{object};
  request.set_reply_to(subscription)
    .set_deadline(std::chrono::milliseconds(250));

  channel.publish("MyService.Increment", request);

  auto reply = channel.consume_until(request.deadline());
  if (reply) {
    if (reply->status().ok()) {
      is::info("RPC successful");
    } else {
      is::info("RPC failed");
    }
  } else {
    is::warn("Deadline exceeded!!");
  }
}
```

## Development

### Dependencies
Run the bootstrap script to install dependencies.

```shell
./bootstrap.sh
```

### Tests
Run the build script to compile and test the library.

```shell
./build.sh
```
