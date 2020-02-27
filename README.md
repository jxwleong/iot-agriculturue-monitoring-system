[comment]: # (Start of Contents)
## <a name="content"></a> Contents  
1.  [What is this repo about?](#repoIntro)
2.  [Requirements for this repo](#repoReq)
3.  [Hardware setup](#hardSet)  
       1. [Client node](#client)  
            1. [Sensor node](#sensor)  
            2. [Relay node](#relay)  
       2. [Server node](#server)  
       3. [Power measure node](#pmNode)  
       
7.  [Results](#result)
       1. [Sensor node](#sensorRe)
       2. [Relay node](#relayRe)
       3. [Server node](#serverRe)
       4. [Power measure node](#pmRe)
8.  [References](#refer)  
9.  [Appendices](#appdix)
    1. [Setup of Arduino IDE](#setArduino)
    2. [Setup of ThingsBoard](#setThingsBoard)  
    
[comment]: # (End of Contents)



[comment]: # (Start of What is this repo about?)
## <a name="repoIntro"></a> What is this repo about?
This is an project to monitoring the soil moisture of the soil of plant, if the soil moisture falls below optimal level, then an automated irrigiation system to turn on the water pump.  

[comment]: # (End of What is this repo about?)



[comment]: # (Start of Requirements for this repo)
## <a name="repoReq"></a> Requirements for this repo  
**Hardware**
1. NodeMCU x 2
2. WeMos D1 R2 x 2
3. 5V Relay x 1  
4. AC Power Socket Rocker Switch 3 Pin x 1
5. 16AWG Silicone Wire x 1m (optional for different colour)
6. 3 pin wall socket x 1
7. An enclosure box for relay and MCU x 1
8. Soil Moisture sensor x 2
9. DHT11 Soil and Humidity Sensor x2

**Software**
1. [Arduino IDE](https://www.arduino.cc/en/Main/Software)  
2. ThingsBoard (Demo version/ Professional Edition if you want to export the data to local PC)  

For the schematic of this repo, please click [here](#BDFull).  
  
[Back to Contents](#content)

[comment]: # (End of Requirements for this repo)

 
 
[comment]: # (Start of Hardware Setup)  
## <a name="hardSet"></a> Hardware Setup
The project consists of 4 hardware nodes which are sensor nodes, relay node, server node and power measure node (just to test power consumption). Figure below shows the fully integrated system for this project.  

<a name="BDFull"></a>  

<p align="center">
   <img src="https://i.ibb.co/C2PzJq8/AP-UOt6s-XOv7hcomx6-a-Cx-LBGX-F-s-Ge-IZLc-QI5ic7cpt-D41-YTys-REdvf-AOPL4531-Iyhiur-Km-Ro-Cz-Ir-S2ro-MAOj-P8f-Cx-Oq-Abqq-Dgs-PEgm-CFRNRGu8v-HXjm-Fpy-CLXhx-Zj-Vyc3w.png)">
</p>  

### <a name="client"></a> Client Node
This project consists two type of client nodes which are sensor nodes and relay node. These client nodes are setup to communicate to the IoT platform (ThingsBoard) via server node.  

#### <a name="sensor"></a> Sensor Node
The sensor nodes are incharged for acquiring the sensor data and sent it to server node via MQTT. Moreover, the can also receive command from ThingsBoard using MQTT.  

##### Flowchart of Sensor Node
![](https://i.ibb.co/VNRgZSc/ka-NMp1snsh-MYrh-F-o-L-d-UT-MT2-Sg-EA5z-Jk-IW5-Fu9jb7bk-HTp-AOtzvym0-EMAb-Kc-PCQPLOqp-XE3-QGGv-Ka-Sr-JDSIBj9tdxx-Q-HEFKu-Ze3c0-Ac-Pg-VUzo-TGp-WRv0-D7-P5-Vb4h3-SJk-XI.png)  

##### Breadboard View of Sensor Node
<p align="center">
   <img src="https://i.ibb.co/j5MsXsd/M6-Iv-G6-GPo-S4o-G4-G4hq-MTt-FKUtk-LH4-WHpbzdf-Xx7-C1-ELN7s-JC0-I-f-I-n-QCjmcu-Wa-O26-XASBFa-QNE-H4s-AWqxd4-MPHgc-p7-XTXf-D1h.png">
</p>  

#### <a name="relay"></a> Relay Node
The relay node responsible to turn on the relay to power up the water pump if the soil moisture is below optimal. It toggle by receiving 'ON' or 'OFF' command from server node using MQTT. 

##### Flowchart of Relay Node
![](https://i.ibb.co/pJHzLf5/rn40-XHB7-J5a-FQGo-VLau-BWk1a-Cr-JQcy-Fvlx-I2-LVtbf-Xdbml-7j-Veq-JK4h3x4-Xkfml-Ysbi-Bp-ELI6jf-Bo-IDw50g8fun5-Qtjz-VQ3-Tn-A6z8-Lp9u-Zyn-Co-L43-St2u2s9z-Sz-Pfvs0g-Cr-Kj-Y.png)  

##### Breadboard View of Relay Node
<p align="center">
   <img src="https://i.ibb.co/bvNbRzX/3q7-Wxyfvjq-Xe-JOr4y-Kwfc-P5o-a-Rs-R-PLRMl-L-Voz-ENwd-Y5e-Xu-JRq-Qocnb6-9-XDgqg-H8-m-M0fe5-FJzb-Fhs0-T4v-Ti8-J9-r7ena-Aqst-Bz-Gf-Dqm4bh-Df-QITUt-Fcxc5hz-Z2-K-b-Nd07o.png">
</p>  
    
[Back to Contents](#content)


### <a name="server"></a> Server Node  
The server node was given several tasks when the system is running:
- Upload the data received from sensor nodes to ThingsBoard.
- When soil moisture data was received, the sensor node need to make decision whether to turn relay 'ON' or 'OFF' by sending command.
- Sent command received from RPC Remote Shell on ThingsBoard to client nodes using MQTT.

#### Flowchart of Server Node
![](https://i.ibb.co/ZNx0bnj/i0t0a-Lj-RHe-W3bnvj-J5i-AYCM3-W1-Wbc-EOZl-Yac1c-Kz-A27a-Eq-X-p-VPSqgl-S3y-FFIKi4-A5y-ZLJb0-ZATCqd-WMASqx-Zkkvzgy-QQEBI0-Vwl-B.png)

#### Breadboard View of Server Node
<p align="center">
   <img src="https://i.ibb.co/TbN57jq/v6s4-DIq41f-Y5d-Fkk-Nx-Az4u-Iax-JVjh7cwa-HSh-N6-Lmb-Gic-z-E5-MR-ol-CK9bg-r-Ph-Li-PH-Iy-Xx7k-G4-Xb9-Vi-Jf-NN26h-CCHz-Pd-Eo-VT-pko-YQxr-NG979m-PASw8rlk-CY-l-O5p-Nuv-NU9f4.png">
</p>  
    
[Back to Contents](#content)


### <a name="pmNode"></a> Power Measure Node 
Since the sensor nodes are design to acquire data remotely, it is important to know the power consumption of the sensor nodes. With the power consumptions of the sensor nodes, the battery life span can be determined. Moreover, suitable energy harvesting system can be determined for recharge the battery of sensor nodes. Thus, this node is created just to test the power consumption (Will not used in the field).

#### Flowchat of Power Measure Node
![](https://i.ibb.co/rtFYwg4/jd-Pkltuk2r802-ZBow-V78m6-G0-v-Lq9ql3m-DNWFu-TFgbdb1x-SORAB-2fm-LBKn5w-JNTL9a-QDGZp-Oa-om-Ayn8-G43-Rd-JHCDm5-Hf-TVo-WIdqi-L.png)  

#### Breadboard View of Power Measure Node
<p align="center">
   <img src="https://i.ibb.co/8PxqHnr/KWz9-UOa8v6-FExts-Hi-TULtq8w-Orp-Q5z-JRfy1-C0-G3-MSc-K5-RTTx-Ukusgt-A0k-Kubdh-F6z1p-Ey-YBRPVTEr-TUc-Oxom-TE3-XRHnfh-VMq-JVP-Uyrlvcb-F8q-IGg-VSsm-Cqsy-YDYJYex6-Rd-LLy0.png">
</p>   
    
[Back to Contents](#content)

[comment]: # (End of Hardware Setup)



[comment]: # (Start of Results)
## <a name="results"></a> Results     
### <a name="sensorRe"></a> Sensor Node Results   
#### Sensor Readings on ThingsBoard's Dashboard  
![](https://i.ibb.co/DVXSWKh/Result-Sensor-Data-on-Things-Board.png)  
#### Sensor Readings Export to Desktop (ThingsBoard Professional Edition)  
![](https://i.ibb.co/vsSGchX/IZZd-Dkbv-AVb-VUj3tl-VBET7-Vdcs-2-Oj-Fomuwcua9-Jea-PErrou-Fz9flkjva8o-UR0vt-P5-XCh-ZL1j8l-O0w-Rn2l-Dx-Vhk-u-M0-Tgr9-T6-ZBJhb7-Dka-JQr-Ds-HWidu3g-IU9b0b-Sa7t-WXTK-Yo.png)  
#### Warning When Soil Moisture is not Optimal.
![](https://i.ibb.co/nRF9XbQ/FYP-Sensor-Alarm-Warning.png)

### <a name="sensorRe"></a> Relay Node Results  
#### Relay Off When Soil Moisture is Optimal
The relay will remain off when the soil moisture was above optimal.
![](https://i.ibb.co/TY7pn7B/FYP-Relay-Off.png)

### Relay On When Soil Moisture is below Optimal
![](https://i.ibb.co/2ZKWF2F/FYP-Relay-On.png)
### <a name="sensorRe"></a> Server Node Results  
### <a name="sensorRe"></a> Power Measure Node Results  

[comment]: # (End of Results)



[comment]: # (Start of References)
## <a name="references"></a> References    
    
[Back to Contents](#content) 

[comment]: # (End of References)



[comment]: # (Start of Appendices)
## <a name="appdix"></a>  Appendices  
### <a name="setArduino"></a>  Setup of Arduino IDE
1. Download and install Arduino IDE from https://www.arduino.cc/en/main/software.
2. Open Arduino IDE, go to File > Preferences.  
![](https://i.ibb.co/7SpPr2T/G7-SUBSht-Kmu-Y-n-JO30n-Cfy-Sinh-MHAllbr-S8odn-WEu-Jngc-Gs-AKj-Is-Ey-V2nx3-T2-QB8mn8-Of-Hc-W1-A8-Wge27iqvzmz-W3-GSn6-Zd8-Ccul-RUa-U.png)
3. At Preferences, insert http://arduino.esp8266.com/stable/package_esp8266com_index.json at Additional Boards Manager URLs      
![](https://i.ibb.co/BKTdJp3/ekbk-YI9tmfjp5-Ad95s-Qm-owr-AWfr-Dv7t0z-Vs-V-6-5-9nm4-LP38w7yg7f-Hyi-C4-L012-Fg-BT-c-BGOkfv-OCyh-c5-FUPBq9r-Yn-KBp943-Pi2.png)
4. Go to Tools > Board>Boards Manager.  
![](https://i.ibb.co/KVFFH5J/O6d-Te-OLKCZ-sslvk-YUUd-IKhjaq-Exq-QCbk1ti320-EPHsqpeo-XS0q5-Rhk-TQ3gw0-Ki-Vl-Cb331-Zdz-A8-I2-X4-RQOTcmiq8-O-r7xj-Rt-Zi18-Z6-B7.png)
5. At Boards Manager, type “esp8266” at the search bar and look for “esp8266by ESP8266 Community then install for the latest version”.  
![](https://i.ibb.co/HPTcJgm/M7-Giv-Wv-RM-O0j-Bfnd4-Sfvbv3-Esa-WIv1-GNkt-ATs-Sfs-LIj-Fpn-LSz-B7-Bgy-BJn-K5g743e-Wq-JZJSw-LEQw-Lgrd9-HNRff5-Oh-QTZP17-MCdhv8w-Lp-IUCFbr-MWh9c53-V7bh-Drz87-GM-Gsckr4.png)
6. After a few minutes (varies on internet speed), the library should be installed.  
![](https://i.ibb.co/3Yzjpdc/4yks-ZCa-Mh-GX3-NRb-Qx-ZTp-Y75-DDE0-CLFVw-Ve-WYBITZgnv4-ZSKku-Aba53-Ku-T-k-UOE0h-WTRz-FBw-BC5-Ubl0-Hts-Ow-DM9egpj-Sb-a-KGRp-NHchd-ENRx-Tpy-SPn-Eq-ITq-Dh0-Cyj-oyw-Yra-Ygc.png)
7. Download the CH340 driver from https://sparks.gogo.co.nz/ch340.html based on your operating system, unzip the file and run the installer.   
![](https://i.ibb.co/hWhXgZh/2-Ci2p-H8tz-Rnz-Ejy8-Nt-K2k-PVzfkxs9k-Foawy-Svbn-ZWw1-LQ10b3-Ali-Pcogb-KFE2-Ful-K5yhk-Asuq-FAFRk-CEZog4-OECp-P4-al-LHf-Mi-QQz-Q7-QCMD9-Bo-XAPl-jt3prr-HBWFRP658-A2-6-A.png)
8. Download the CP210x driver from https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers based on your operating system, unzip the file and run the installer.   
9. Make sure to use micro USB data cable for connection between MCU and PC. Go to Device Manager > Ports (COM & LPT). In my case, the COM port is COM7.  
![](https://i.ibb.co/fttfKhn/pok-Ufz-NK9-Kyh8-OEbs-x-R5m-CV2-A8i-0a-IX5po-FATlj-Jw-EToo-F-MGrfq9-VVqt-Fw-OFGC1-Ks4z-S-o-BN5l-Hlqm-Uu-QYSJo-Jgdt-UGk-IYwrq-AEwby-Ve-dctl-QC-BNAd-MLw-LXt-RJn-A2-CE2c.png)
10. Go back to Arduino IDE, go to Tools > Board, choose “LOLIN(WEMOS) D1 R2& mini” for server and “NodeMCU 1.0 (ESP-12E Module)” for clients.
![](https://i.ibb.co/rmZMHm3/i-Dke-Nqj-Jal38-Dyh-Dphg-ZA6y-AS8-Wd5-X2-H27-n-I5i-OKBw-PLOz-B8vx3-Wu-AAE-v-Kx-CXCERm8l-MD-x-Hzib8-Yd6-FTZEUHLXKMv-3-Nn-Sk-I-py8-Py-AK0-OReegq-ZHel-K8q-OHhwdz-KHC-KRA.png)
11. At the same section, select the COM Port found at step 8.

### <a name="setThingsBoard"></a>  Setup of ThingsBoard
1. Sign up an account and log in at https://demo.thingsboard.io/login for demo version and https://cloud.thingsboard.io/login for professional version.
2. Go to DEVICES  then click on the ‘+’ sign on the bottom right and select ‘Add new device’.  
![](https://i.ibb.co/dJYp23W/n-CEr-ZIjo-Pj7ap96-D3lo-Zmg-X5-WH-Ee5-YNqxdz-Cfy-BL7-VGYr-Kdk3-Rh-Hi9x-EX2-SCZNk4-Ua-Pl-Td2u-Nw3-A6zj-Xf-MJPRq01-Fy-FDh-Juj-rd-1-PI25m4k-Xk-OX9r5-Vy-Ol-Qai-F5k0-Many-M.png)
3. A window will pop-up, key in the suitable Name and leave the Device type as ‘default’ then click  ADD.  
![](https://i.ibb.co/p0CZs61/x-Ca4yl2ynpo-T6v9-Bu-ESlv3v0hy5-Mb-Hdx-LO8-CURrl-LSrzh-NHb-M-AZ0-I1b4-Zg-Xvj-NNcq-gi-HGI7-Rm-Ngttkn8h3g-Vys-M6-I2-CFtht-Iwdgxq4-WHxj10u-RVT3-DYik-Tg091p8-Nm-Tx-H2o8.png)
4. Click the created device, then copy the DEVICE ID  and ACCESS TOKEN to a notepad. It will used later on.   
![](https://i.ibb.co/7V6kt0y/Wpyv-Tzvr-WX4a-C9x8-Jjp-GDK3h-I8t-KMt28-Bu-SNZ51-GPfjg9v-Czc-FWe-ALxzafy-SKZ0n-Ev4wf-RA7-Im-EOLZHndg-Kx4-EZKjv5d0-Ylk-ND6-YKr-Ksii-Xni-Bz-N5er-Pz4-TJN7-PIj-Ma-RZ7-LICs.png)
5. Go to Assets and repeat step 2 and 3. Leave the Asset type to ‘location’.  
![](https://i.ibb.co/KFhSQ1P/HNhj-KGZZJ6-LL3-Bdw-Aj-GO2-Lmh7z-JYPJml999fj5-Jk-GWw7lsa-KAQTT2u-M4-Eg-Ld-O6-Ki-W6t1-LDj-T3-Qj-Acb-Uf-Kp7-XLOKm-Vv-Nu0l-Kprn-Ol3pq-Un7x9-Rjga-XH48xyy0-Dz-Ryi-ns-O5j-ECw.png)  
6. Click on created Assets, go to RELATIONS then click ‘+’.  
![](https://i.ibb.co/gFdhXZY/l-WEP3k-JUlafch9-I7-XFXnb-Kgzd-Zql-Scq5-L4jh-GZKd-WSg2-F5g-VQVMZWwu-P2-Ah-Oo-JCry-A0-Kl-Cq6vvs-E9-DMlf-Gn3-Pdly-Nj-Bjt6-FH9-K0-W6-Ga-Oshg5ei-QSpva-Nkw-j7-MUb-QXLBLr-U8.png)
7. A pop-up will shown, select the Type as Device and Key-in the Device name just created at step 3 then click ‘ADD’.  
![](https://i.ibb.co/q98wsG6/1-R3-K4i-Eppel57-Fl-Zh9-TAJYq-QXc-ATrjyo-Jj-0q-BPPj-Ofb-K7-L3-Jd-Oa8-MFH5i4-Yjo0gh-ZWGt-FQFPsz32-Pi-P27-Wk-Sspw-Yj-W5-YLHao5e-Vm5-Yi-Rdqmjsw-Fs-Oz4-J5-Nm-MLk-WIRJS8n-Ta94.png)
8. Go to DASHBOARD and create a new dashboard.  
![](https://i.ibb.co/Z8VXd0G/JYw-HQU2-JRi-FG8h-Zcpafw-Vy-SISSVlc-RPUn-EEg-Z4zi-Cc6ax-Gbf9-VY0-O9-C9i4p7l-Kkx-MPts-Eta-Zvi-Ond89jwj5nwwq-Sswyy-UMEN4-U2-Vg-MWYj-UB6-XLD4o-MDt-YBB3q5x-Jtkh-Hqzf-Tg.png)
9. Click on created dashboard then enter into edit mode.  
![](https://i.ibb.co/1Tt1V4W/M4-Foiwn9fw-JI1-DFgs-Ve-W6-SV6v1-lm-SOIhs5o-Gk-Rs-C8-EBJIXHE4s92b3rzl-YPOH53-Ex-YYfrtirix-cp-JInl-Owa-UJ-g-Nm0-XDEnj-Yc-ZJU3-Cj-Oj-E6qf-HNkz-YO3s-LEps-Cp-Gn-Esivzb-A.png)
10. Click ‘Entity aliases’ then click ADD ALIAS  
![](https://i.ibb.co/8jqmQKb/85a-Zyr-R-X8-GIR52p-Wnc-Dzbu8-Ymv-Jjy-Fu3m-SFFpyl-WG7r-MIy-Het5k-ASPrdc38-Hd-Dme-Te8-V9-ZYYm-Hp-Uj8-NCzw-Ys-W-hdog-EZa-VRk-HYM6gbh4-FF-l-Ii-Ln-EKDgz-Jr-Jlpt-Aa-HVASKSk.png)  
11. A pop-up named ‘Add alias’ will shown. The Alias name can be set to any name convinient but the Device, please select the device created earllier. Other options leave it as  shown at figure below. After click ‘ADD’ then click ‘SAVE’.
![](https://i.ibb.co/v11dzBv/Vpn-CQyb-LKfc4-HC6-LWGy-TPls8s7-G-e338-Ujh-MO3i-Gs-C81-ZHt-ADqrs-Zt-Ob-Yb91tt-IGBYy-KWxm5pz-Eyn-v6yde-IFrn0ajqn4re-Z-0-G1gq-Ml8z-K76d-Jjxoq-De-KU8q-Me-L6-Qkq-Rg3-Qwo.png)  
12. To add any widgets to the dashboard, just click Add new widget > Create new widget. Then select the widget type and click any desired widget.  
![](https://i.ibb.co/289FgBF/gd8-Xwb-Pl-Mf-I6ldi2-Bpzd-Kj30-UQOys-Ysn-X3k-K1-MGRj-3hw-TN-s-B5-GL8t7-KIMJH4-E57-U8w3-wsss-Xeg-URU1-Ev6-ITMlklv-KRtm-Vetg-SRY94-Sngz-Scu-EW-8xl-UAEDh-Wd-P7br6-BGk.png)
13. Select the Entity alias as alias created at step11. Then, type the parameter name desired and press ‘ENTER’. Then click ‘ADD’. The similar steps can be applied for other widgets.    
![](https://i.ibb.co/0V7jyNt/k-Epy1n-RDfm-DD68-f-MX77-Pfi-LBq-Dt4a3-LDqq-Kc-Larap8z-YNYtj-Jr2-Vog8-Js9c-UPxw-Tum-IKId-I4r2qw-XSx-Wg0-FZ-8o-A7qx-YBq-PHAPU6-J.png)  
    
[Back to Contents](#content)

[comment]: # (End of Appendices)
