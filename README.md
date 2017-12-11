# restdbxx
[![Build Status](https://travis-ci.org/baskeboler/restdbxx.svg?branch=master)](https://travis-ci.org/baskeboler/restdbxx)

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

## JsonClient

Utility class created with the purpose of making GET requests to remote servers less painful.

### usage

You initialize the class with the requests details and then you invoke the following
method:

> `void JsonClient::fetch(proxygen::HTTPMessage *req, folly::Promise<folly::dynamic> &promise)`

Pay attention to the promise that the function receives, you should get a future from that 
promise before calling `fetch()`.

Here is an example:

```cpp
folly::Promise<folly::dynamic> promise;
  auto f = promise.getFuture();
  folly::EventBaseManager::get()->getEventBase()
      ->runInLoop([comic, p = std::move(promise), this]() mutable {
        auto req = new HTTPMessage();
        std::stringstream ss;
        ss << "https://xkcd.com/";
        if (comic) {
          ss << comic.value() << "/";
        }
        ss << "info.0.json";
        req->setURL(ss.str());
        req->setMethod(HTTPMethod::GET);
        client->fetch(req, p);
      });
  f.then([this](folly::dynamic &comic) mutable {
    ResponseBuilder(downstream_)
        .status(200, "OK")
        .header("RESTDBXX_QUERY_TIME", std::to_string(client->get_elapsed()))
        .header(proxygen::HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE, "application/json")
        .body(folly::toPrettyJson(comic))
        .sendWithEOM();
    return;
  }).onError([this](const std::runtime_error &e) {
    ResponseBuilder(downstream_)
        .status(500, "unexpected error")
        .body(e.what())
        .sendWithEOM();

  });
```
