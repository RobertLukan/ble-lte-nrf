In 2023 my and wife have decided to rent 40square meters of farm land so we could grow vegetables. Of course, I was interested also in more technical aspects of growing vegetables. In 2022 I came across a very interesting project b-parasite https://github.com/rbaron/b-parasite that measures soil moisture and broadcasts data over BLE to a dedicated receiver. I have been running several soil probes for about 2 years on my balcony. 

I wanted to expand this idea to rented farmland. I had several challenges there, such as no Wifi and power. So I had to use LTE and battery power. This project was developed prior to my already posted project of NRF camera project(which is still in development).  For this use case, I used Thingy91 from Nordic. Thing91 combines nrf9160 and nrf52840. I managed to listen to BLE broadcasts and convert them to UDP packets sent over LTE. I have created also a dedicated server(Python-based) that listens to UDP packets and automatically creates Home Assistant objects. 

I had several issues combining the software needed for nrf52840 and nrf9160. Initially, I installed quite an old version of software needed for 52840 as I could not get the LPUART example working. Unfortunately, that version had a bug that I did not have time to look into it. The workaround was to remotely reboot Thing91 if it got stuck (and it did a couple of times).  For this reason, I have decided to postpone posting this project online. Luckily, Nordic managed to get that bug fixed and I ported my software from SDK 2.3.0 to the latest(2.5.2)

I am not really a software developer, so my code is not really the best, but it works :) One important note, in order to install BLE code on nrf52840 inside of Thingy91, an external debugger is required. I am using nrf9160DK for this purpose. There are still a few bugs that Nordic has to fix, but workarounds are available for now, just take a look at this website for more details: https://devzone.nordicsemi.com/f/nordic-q-a/102456/lte_ble_gateway-fails-with-ncs-2-2-and-thingy-91-rev-1-6-0/440837
Those workarounds are for NRF SDK and are explained here: https://github.com/nrfconnect/sdk-nrf/pull/13407

As the base example, I have used the nrf_cloud_multi_service example. I have added the BLE example(modified to only listen to broadcasts from b-parasite probes) and added a specific code to collect the data from b-parasite probes in a list and transmit it every 10 minutes. This example talks to NRFcloud where FOTA(remote upgrade) can be done. Also, it allows sending GPS/AGPS/Celluar location and it connects to the MQTT service provided by nrfcloud. I needed MQTT service so I could send commands towards the Thingy91. I could implement this feature also on my server, but I found this more convenient. To reduce monthly costs on NRFcloud, I have disabled sending messages to NRFcloud, but I left GPS enabled, just in case someone decides to take this hardware, I "might" be able to locate that person.
The Python server simply listens to UDP packets, decodes them, and posts them over MQTT to my Mosquitto server and Home assistant detects them. 

The code contains 3 parts. Code for nrf9160 on Thing91, the code for nrf52840 on Thingy91 and a Python script that has to be installed on a server with a specific UDP port opened from the Internet. Unfortunately, this will not work behind CGNAT. I have used UDP as it is not power-demanding. This code will not work directly with more generic hardware. Adjustments of the device tree would have to be made. The prj.conf contains some redundant configuration, as I was testing a couple of other components, but I did not have time to clean it up properly.  This project would require more comments, which I might add at a later stage, but feel free to ask me a question. 
