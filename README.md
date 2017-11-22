# restdbxx


## dependencies

the following libs should be compiled in this order:

+  gflags
+ glog
+ boost 
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
  + GET will return a list of endpoint descriptors
  + POST will register a new endpoint, for now the following json body is valid:
    ```json 
    {
        "url": "/new_endpoint" 
    }
    ```
    + more properties in the future.
+ /authenticate
  + POST json like
    ```json
    {
        "username": "some_user",
        "password": "somepassword"
    }
    ```
    + if successful, the service will return a json object with an access token.
    + for protected requests, include the following headers
      + RESTDBXX_AUTH_TOKEN with provided access token string
      + RESTDBXX_AUTH_USERNAME with username
      + if valid, request will proceed, otherwise it will be rejected.
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