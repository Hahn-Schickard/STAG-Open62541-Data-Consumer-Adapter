#ifndef __DATA_CONSUMER_ADAPTER_INTERFACE_HPP_
#define __DATA_CONSUMER_ADAPTER_INTERFACE_HPP_

#include "Listener.hpp"

namespace DCAI {
  typedef enum DataConsumerAdapterStatusEnum {
    INITIALISING,
    RUNNING,
    STOPPED,
    EXITED,
    UNKNOWN
  } DataConsumerAdapterStatus;

  class DataConsumerAdapterInterface : public Model_Event_Handler::Listener {
   public:
    virtual void start()                                                = 0;
    virtual void stop()                                                 = 0;
    virtual DataConsumerAdapterStatus getStatus()                       = 0;
    virtual void handleEvent(Model_Event_Handler::NotifierEvent* event) = 0;

    ~DataConsumerAdapterInterface() = default;
  };
}   // namespace DCAI

#endif   //__DATA_CONSUMER_ADAPTER_INTERFACE_HPP_