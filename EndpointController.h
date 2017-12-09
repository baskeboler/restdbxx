//
// Created by victor on 11/11/17.
//

#ifndef RESTDBXX_ENDPOINTCONTROLLER_H
#define RESTDBXX_ENDPOINTCONTROLLER_H
#include "BaseRequestHandler.h"
namespace restdbxx {
class EndpointController : public BaseRequestHandler {
 public:
  ~EndpointController() override = default;
  void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;
  void onUpgrade(proxygen::UpgradeProtocol prot) noexcept override;
  void onEOM()noexcept override;
  void requestComplete() noexcept override;
  void onError(proxygen::ProxygenError err) noexcept override;

  static const std::string &ENDPOINTS_PATH();
  void processPost();
  folly::dynamic get_endpoints_dynamic() const;
};

}

#endif //RESTDBXX_ENDPOINTCONTROLLER_H
