#ifndef __DATA_CONSUMER_ADAPTER_INTERFACE_HPP_
#define __DATA_CONSUMER_ADAPTER_INTERFACE_HPP_

#include "Listener.hpp"

/**
 * @brief Data Consumer Adapter Interface
 * This namespace containes all of the information, required by the Information Model Manager. 
 * @author Dovydas Girdvainis
 * @date 04.12.2019
 */
namespace DCAI {
  /**
   * @brief State enum for the Data Consumer Adapter implementations
   * 
   */
  typedef enum DataConsumerAdapterStatusEnum {
    INITIALISING, /* Set in DataConsumerAdapterInterface::start() routine as its initial step. During this state all of the necessary configuration routines are executed */
    RUNNING, /* Set in DataConsumerAdapterInterface::start() routine as its final step, if it succeded. During this state the adapter is interacting with the Information Model Manager */
    STOPPED, /* Set in DataConsumerAdapterInterface::stop(), if it succeded. During this state, adapter stops all of the interactions with the Information Model Manager */
    EXITED, /* Set in DataConsumerAdapterInterface::start() routine as its final step, if it failed. During this state, adapter stops all of the interactions with the Information Model Manager*/
    UNKNOWN /* Initial state, before DataConsumerAdapterInterface::start() routine is called. During this state, adapter does not interact with the Infromation Model Manager */
  } DataConsumerAdapterStatus;

  /**
   * @brief Generic Interface for all Data Consumer Adapter Implementations
   * 
   */
  class DataConsumerAdapterInterface : public Model_Event_Handler::Listener {
   public:
    /**
     * @brief Starts the adapter. 
     * 
     * If succeded, sets the internal state to DataConsumerAdapterStatus::RUNNING
     * 
     */
    virtual void start() = 0;
    /**
     * @brief Stops the adapter.
     * 
     * If succeded, sets the internal state to DataConsumerAdapterStatus::STOPPED
     */
    virtual void stop() = 0;
    /**
     * @brief Gets the current state of the adapter
     * 
     * @return DataConsumerAdapterStatus 
     */
    virtual DataConsumerAdapterStatus getStatus() = 0;
    /**
     * @brief Handles incoming event from Information Model Manager
     * 
     * @param event 
     */
    virtual void handleEvent(Model_Event_Handler::NotifierEvent* event) = 0;

    ~DataConsumerAdapterInterface() = default;
  };
}   // namespace DCAI

#endif   //__DATA_CONSUMER_ADAPTER_INTERFACE_HPP_