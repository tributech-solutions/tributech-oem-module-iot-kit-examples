# Tributech OEM Shield sample with Arduino UNO R3

## Develompment Kit Setup:

### Pre-Conditions

To be able to use the Tributech OEM shield sample the user needs to assure that all  of the following pre-conditions and resources are at hand.  

* Tributech [OEM Shield](https://tributechwebcontent.blob.core.windows.net/tributech-web-content/Specsheet_2022_OEM.pdf)
* Arduino [UNO R3](https://docs.arduino.cc/hardware/uno-rev3)
* Infineon [My-IoT-Adapter](https://www.infineon.com/cms/de/product/evaluation-boards/my-iot-adapter/)
* Infineon [S2GO-Pressure-DPS368](https://www.infineon.com/cms/en/product/evaluation-boards/s2go-pressure-dps368/)
* Antenna with U.FL connector (LTE CAT-M capable)
* micro LTE SIM card
* USB A to micro USB cable or 5V power adapter with micro USB
* Username and Password for Tributech Web portal access

### Setup your development kit

With the following steps the user is able to assemble and start the sample kit. 

1. Connect your OEM shield to the Arduino UNO
![Connect your OEM shield to the Arduino UNO](images/uno_oem.png)
2. Put the SIM card into the OEM shield
![Put the SIM card into the OEM shield](images/uno_oem_sim.png)
3. Connect the antenna to the OEM shield
4. Stack the My-IoT-Adapter on top of the OEM shield with the S2GO-Pressure-DPS368 sensor
![Connect the S2GO-Pressure-DPS368 sensor with the My-IoT-Adapter](images/adapter_dps368.png)
6. Supply power to the XMC Relax Kit via the micro USB power connector
![Supply power to the XMC Relax Kit via the micro USB power connector](images/arduino_uno.png)

When all steps are completed the green LED of the Tributech OEM shield will blink when the initialization of the board has been completed successfully. 
The blue LED will start blinking when a successful LTE CAT-M connection is established.

When all the above steps are completed follow the instructions in [Verify Device2Cloud Communication](#verify-device2cloud-communication).

### Verify Device2Cloud Communication

The verification of the connection and communication can be done in the Tributech web portal.
The user has to access url of the node which is linked to their device and complete the login.
The address always follows the following format:

    node-name.dataspace-node.com

The "node-name" is dependent on the node which is linked to the device. The node-name as well as the login credentials will be provided by Tributech via email.
The main page of the web portal shows multiple sections all with their own information, but for the verification the user needs to look at the left-hand side an click on the "Agents" tap.

![List of all Agents linked to node](images/AgentManagementTap.png)

This tap lists all agents which are linked to the node. They are listed with the following information:

| Name | Type | Status | Key Storage Type | Proofkind |
|------|------|--------|------------------|-----------|

Name depicts the different names of the agents. Type lists the device type of the respective agents. Status shows if the agents is online or offline. Key Storage Type will show the user where the private keys are stored and ProofKind depicts which key format is used for the proof signatures.

Here the user has to select their own device and click on it. The names of the device will be provided by Tributech via email and will be depicted either on the device box or on the device itself. This will lead the user to the "Agent Management" page. An example of the agent management page is shown in the following picture:

![Agent management page: agent: Tributech OEM node:dev-node-a](images/TributechPlatformAgentManagement.png)

Since the connectivity already is verified when the device is listed as online the next step is to verify the device communication. This can be done via clicking on the on of the listed streams.
This action will show the stream information and it's associated values link in the picture below:

![Temperature stream of an agent](images/TributechPlatformAgentStream.png)

The stream values will be depicted in a graph or table format below the stream information.
## Get started with your project

### Setup Dev Environment

To be able to send individual values/streams via the Tributech OEM shield these values have to be transferred to the device via a UART connection. In this sample the Arduino UNO R3 Kit is used.

In this sample kit the one normal sequence consists of a sensor gathering call from the Arduino Uno R3 Kit to the S2GO-Pressure-DPS368. After that the sensor values are packaged into a JSON and send to the Tributech OEM shield.
If a user wants to add their own stream to the device a software change on the Arduino Uno R3 Kit has to be conducted.

The pivotal point here is the Arduino Uno R3 Kit so the user has to setup the Arduino IDE to be able to engineer a custom software for this board.
The Arduino IDE can be downloaded on their website:
    
[Arduino IDE](https://www.arduino.cc/en/software)

Install the IDE and open the project file. Install the two Libraries Base64 and DPS310 (DPS368) and insert the correct ValueMetaDataIds in the code.

![Install Base64 library](images/LibraryBase64.JPG)

![Install Adafruit DPS310 library](images/LibraryAdafruitDPS310.JPG)

![Replace ValueMetaDataIds ](images/ReplaceValueMetaDataIds.JPG)

To flash the project you have to unplug the OEM Shield.

If you flashed the microcontroller stack the OEM shield again on the Arduino UNO and repower it. You cannot open the usb serial connection because the Arduino UNO has only one serial channel and this one is now used for the OEM Shield. 


### Configure OEM Module

The Tributech OEM shield only acknowledges values provided via UART which have a valid ValueMetadataId. These ValueMetadataIds are generated automatically when a new stream is added to the Twin configuration of the OEM shield. The Twin configuration is a electronic representation of a IoT-device, it contains the configuration parameters like metadata, configurations and conditions of the device and their different correlations to each other. Parts of this Twin configuration can be altered by the user like adding sources and streams to the configuration. Thus the user has to configure the OEM shield via the Data-Space-Admin. For more in depth information on device twins follow this link Twin "[How to use digital Twins for IoT devices configurations](https://tributech.io/blog/digital-twins-for-IoT-device-configurations)".

The configuration is sent to the OEM shield via a MQTT connection which means the device has to be online for a configuration update. If the device is online a new tap can be seen in the Agent management page called "Configuration". This is depicted in the picture below:

![Agent management page Configuration tap](images/TributechPlatformAgentConfiguration.png)

When the configuration tap is accessed the first action is triggered and the configuration is pulled form the linked OEM shield. This configuration is then depicted on the webpage:

![Tributech OEM shield Twin configuration](images/TributechPlatformAgentConfigurationPage.png)

Depending on the location where the OEM shield is stationed and the LTE CAT-M connection the configuration pull can last several seconds. 
When the current configuration is shown in the web portal the user can start changing the configuration. The most important part of the configuration change is to add a custom stream.
To be able to add a new stream first the user should add a new source.
To add a new source right-click on the device name and choose "Add Source". Furthermore search for "Embedded Source" this is the only source which can be added to a OEM shield device all other sources will be ignored. 

![Add additional source to a pre-conditioned device](images/TributechPlatformAgentAddSource.png)

After the source is added the name of the source can be changed on the right-hand side. The changes have to be finalized by clicking on the "Apply" button on the bottom of the configuration. If the changes are done a stream can be added to the source. Again via right-clicking on the source a dropdown menu is shown. In this menu navigate to "Add Streams" and use the only available option "Embedded Stream". The configuration of the stream follows the same principle as the name change of the Source.

![Add stream to source](images/TributechPlatformAgentAddStream.png)

The last change which needs to be done is the addition of the "Value change options". Adding these options is shown in the following picture. The "value change options" consist of three values: PMIN, PMAX, ST. The PMIN value depicts the time frame which has to pass until a new value can be provided to the OEM in seconds. This value has to be at least 10 seconds. **The PMAX and ST values are at the moment not supported by the OEM modules, but will be added in a future update.** The preloaded software will commit a pressure and temperature value each 5 seconds if a connection is available and the sample kit is assembled in the right manner.

![Add value change options to a stream](images/TributechPlatformAgentAddValueChange.png)

With these changes the configuration can be send to the OEM shield via the "Apply Configuration" button.

### Provide telemetry data via UART

If the OEM shield configuration is done, the user is able to send data to the device via the UART connection. The command used for this procedure is "ProvideValues".
A complete "ProvideValues" command looks like the following.

```json
{
   "TransactionNr":14,
   "Operation":"ProvideValues",
   "ValueMetadataId":"10101010-2020-3030-4040-505050505051",
   "Values":[
      {
         "Timestamp":1615879096949000000,
         "Value":"MZiz"
      }
   ]
}
```

Explanation:
- The `TransactionNr` is a continuos number which will be given back in the reply to be able to synch the command and the OEM reply. 
- The `Operation` depicts the command which should be conducted by the OEM shield. 
- The `ValueMetadataId` links the send timestamp and values to the respective stream. 
- The `Timestamp` object can be filled with a timestamp in microseconds. If filled with a microseconds value the time is added to the starting date of 1.1.1970. If the timestamp is left at zero the OEM shield attaches the latest timestamp when the value is received.
- The last object is `Value`, this object holds the value which will be depicted in the web portal. The value linked to the ValueMetadataId has to be encoded Base64. Here the user has to be careful since the value ahs to be encoded accordingly. The Base64 value for 16.5 is "AACEQQ==" if it's encoded as float. If the value is treated as a string the encoded value will be "MTYuNQ==". The value should be encoded according to the configuration of the stream. If the value gets submitted in the wrong format this will lead to unexpected behavior in the web portal, because it will be depicted in the wrong format.

### Consume data via explorer or API

The values presented to the Tributech DataSpace Kit can be consumed in two ways. The first way is get to the data saved by the DataSpace Kit is to access it via the explorer. Here the data-points can either be viewed in the form of a graph or in form of a table. In the "Agent Management" window of the different agents. Each Stream has its own Graph and table and also its own data audit page, where all the proofs are automatically validated, but can also be validated manually. A more thorough documentation can be found in [Tributech Docs Stream Explorer](https://docs.tributech.io/docs/consume_data/streamexplorer/).

The second way is to access the data via the API functions. A complete documentation for the APi functions can be found at [Tributech Docs API](https://docs.tributech.io/docs/consume_data/api/).