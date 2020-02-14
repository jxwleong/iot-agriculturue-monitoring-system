## Contents  
1.  [What is this repo about?](#repoIntro)
2.  [Requirements for this repo](#repoReq)
3.  [Schematic](#schematic)
4.  [Client node](#client)
    1. [Sensor node](#sensor)
    2. [Relay node](#relay)
5.  [Server node](#server)
8.  [References](#refer)


## <a name="repoIntro"></a> What is this repo about?
This is an project to monitoring the soil moisture of the soil of plant, if the soil moisture falls below optimal level, then an automated irrigiation 
system to turn on the water pump.  

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

For the schematic of this repo, please click [here](#schematic).

## <a name="schematic"></a> Schematic
### Block Diagram of Integrated System
<p align="center">
   <img src="https://i.ibb.co/C2PzJq8/AP-UOt6s-XOv7hcomx6-a-Cx-LBGX-F-s-Ge-IZLc-QI5ic7cpt-D41-YTys-REdvf-AOPL4531-Iyhiur-Km-Ro-Cz-Ir-S2ro-MAOj-P8f-Cx-Oq-Abqq-Dgs-PEgm-CFRNRGu8v-HXjm-Fpy-CLXhx-Zj-Vyc3w.png)">
</p>

### Breadboard View of Sensor Node
<p align="center">
   <img src="https://i.ibb.co/j5MsXsd/M6-Iv-G6-GPo-S4o-G4-G4hq-MTt-FKUtk-LH4-WHpbzdf-Xx7-C1-ELN7s-JC0-I-f-I-n-QCjmcu-Wa-O26-XASBFa-QNE-H4s-AWqxd4-MPHgc-p7-XTXf-D1h.png">
</p>

### Breadboard View of Server Node
<p align="center">
   <img src="https://i.ibb.co/TbN57jq/v6s4-DIq41f-Y5d-Fkk-Nx-Az4u-Iax-JVjh7cwa-HSh-N6-Lmb-Gic-z-E5-MR-ol-CK9bg-r-Ph-Li-PH-Iy-Xx7k-G4-Xb9-Vi-Jf-NN26h-CCHz-Pd-Eo-VT-pko-YQxr-NG979m-PASw8rlk-CY-l-O5p-Nuv-NU9f4.png">
</p>

### Breadboard View of Relay Node
<p align="center">
   <img src="https://i.ibb.co/bvNbRzX/3q7-Wxyfvjq-Xe-JOr4y-Kwfc-P5o-a-Rs-R-PLRMl-L-Voz-ENwd-Y5e-Xu-JRq-Qocnb6-9-XDgqg-H8-m-M0fe5-FJzb-Fhs0-T4v-Ti8-J9-r7ena-Aqst-Bz-Gf-Dqm4bh-Df-QITUt-Fcxc5hz-Z2-K-b-Nd07o.png">
</p>

### Breadboard View of Power Measure Node
<p align="center">
   <img src="https://i.ibb.co/8PxqHnr/KWz9-UOa8v6-FExts-Hi-TULtq8w-Orp-Q5z-JRfy1-C0-G3-MSc-K5-RTTx-Ukusgt-A0k-Kubdh-F6z1p-Ey-YBRPVTEr-TUc-Oxom-TE3-XRHnfh-VMq-JVP-Uyrlvcb-F8q-IGg-VSsm-Cqsy-YDYJYex6-Rd-LLy0.png">
</p>
