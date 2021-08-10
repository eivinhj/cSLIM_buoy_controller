.. _mqtt_simple_sample:

IoF: cSLIM 
####################

.. contents::
   :local:
   :depth: 2

The cSLIM buoy controller application is used in the IoF project at NTNU.  

Requirements
************

The sample supports the following boards:

.. table-from-rows:: nRF9160DK + cSLIM shield
   :header: heading
   :rows: cSLIMns


Overview
*********
The cSLIM application is the buoy controller software for the IoF cSLIM module, based on the nRF9160. Running the Zephyr ROTS, it consists of device drivers and software tasks required to interface the buoy hardware and communicate with mainland using LPWAN technologies. 


Configuration
*************

|config|

MQTT Client options
=====================

Check and configure the following configuration options for the application in proj.conf:

.. option:: CONFIG_MQTT_BROKER_HOSTNAME - MQTT Broker host name

This configuration option defines the MQTT Broker host name.

.. option:: CONFIG_MQTT_CLIENT_ID - MQTT Client ID

This configuration option specifies the MQTT Client ID.

.. option:: CONFIG_MQTT_SUB_TOPIC - MQTT Subscribe topic

This configuration option sets the MQTT Subscribe topic.

.. option:: CONFIG_MQTT_PUB_TOPIC - MQTT Publish topic

This configuration option sets the MQTT Publish topic.

.. option:: CONFIG_MQTT_BROKER_PORT - MQTT Broker Port
This configuration option specifies the port number associated with the MQTT broker.

.. option:: CONFIG_MQTT_BROKER_USERNAME - MQTT Broker Username
This configuration option sets the MQTT Broker Username.

.. option:: CONFIG_MQTT_BROKER_PASSWORD - MQTT Broker Password
This configuration option sets the MQTT Broker Password.

.. option:: CONFIG_LTE_CONNECT_RETRY_DELAY_S - LTE connection retry delay
This configuration option specifies the delay (in seconds) before attempting to reconnect to the broker.

.. option:: CONFIG_MQTT_KEEPALIVE - MQTT Keepalive interval
This configuration option specifies the time (in seconds) between each MQTT broker keepalive message.

LTE Power savings
=====================
.. option:: CONFIG_LTE_EDRX_REQ - LTE eDRX enable
This configuration option enables the LTE eDRX power saving.

.. option:: CONFIG_LTE_EDRX_REQ_VALUE - LTE eDRX parameters
This configuration option specifies the requested value for LTE eDRX. 



Logging options
=====================
.. option:: CONFIG_LOG - Log enable
This configuration option enables and disables the Zephyr log.

.. option:: CONFIG_LOG_BACKEND_UART - Enable log to UART
This configuration option enables forwarding the log to UART.

.. option:: CONFIG_LOG_DEFAULT_LEVEL - Log default level
This configuration option sets the default log level.

Configuration files
=====================

The application provides the following predefined configuration files for the buoy controller:

* ``prj.conf`` - For cSLIM


In addition, the sample provides overlay configuration files, which are used to enable additional features in the sample:

* ``overlay-tls.conf`` - TLS overlay configuration file for nRF9160
* ``overlay-carrier.conf`` - LWM2M carrier support for nRF9160 
* ``cSLIM_common.dts`` - Device Tree for cSLIM 





Building and running
********************
The following command builds the application for nRF9160 DK and cSLIM shield:

 .. code-block:: console

    west build -b cSLIMns -p
     
add -p (prestine) at end if this is the first time building, or there are changes in file structure etc. 

Upload using     
 .. code-block:: console

   west flash

Testing
=======

1. Connect USB
#. Open terminal (Lowest COM number of device)
#. Reset the module.

#. Observe that the display illuminates, showing cSLIM at the top and output for different modules, including GPS, LoRa, LTE and TBR.
#. Use an MQTT client like 'Mosquitto' to subscribe to  the broker and verify that IoF messages are received.
   Observe that the development kit publishes all the data to the topic set by CONFIG_MQTT_SUB_TOPIC.

Troubleshooting
===============

If you experience problems connecting to the MQTT broker, make sure the MQTT broker hostname, port, username and password are correct.


Dependencies
************

This application has been tested using Nrf Connect SDK v. 1.5.1, however newer versions might also be supported. 


