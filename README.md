# restdbxx


## dependencies

the following libs should be compiled in this order:

+  gflags
+ glog
+ folly
+ rocksdb
+ wangle
+ proxygen

## endpoints
+ /__users
  + post to this endpoint to create users
  + f.e.: 
  ```json
  {
    "username": "user",
    "password": "pass"
  }
  ```
  + get list of users
  + get specific user, for example /__users/username
+ /__endpoints
+ /authenticate
+ /test_endpoint

## implementation notes
This is an example of implementing proxygen request handlers asynchronously.

```c++
folly::Promise<SomeResult> promise;
auto f = promise.getFuture();
folly:EventBaseManager::get()->getEventBase()->runInLoop(
    [p = std::move(promise), this]() mutable {
        // this is run in a different thread
        SomeResult result = doSomeWork();
        p.setValue(result);
    }
);
f.then([this](SomeResult& res) {
    // we send the response from the original thread
    proxygen::ResponseBuilder(downstream_)
        .status(200, "OK")
        .body(res.getBuffer())
        .sendWithEOM()
});

```