# 🌱 AstroForage

## NASA Hackathon Project

### What is AstroForage?

A plant recommendation app, using environmental data to see how climate and soil data derived from satellites can be used to inform farming decisions.

This project was designed to help farmers to find suitable native plants and crops based on the environmental conditions of their area. It combines various environmental factors and soil data to help turn freely available scientific data into actionable farming insights.

## 🧠 Concept

This project was designed with using NASA's earth observation and environmental data in mind.

The idea was to use environmental data to help farmers find suitable native crops for their area. Native plants are well adaptated to the environment they grow in however it is important to know what climate and soil conditions you have in your local area. This project was designed to look at how environmental data could be used to help this.

## 📊 Data

All data was sourced and chosen through my own research.

Climate data was sourced from the NASA POWER API. This gives an average air temperature and adjusted precipitation data for the chosen co-ordinates. Soil data was sourced from the USDA Soil Data Access API. This provided the soil pH.

This data would be used within the recommendation engine to compare against the ranges of growing conditions for each plant.

## 🛠️ Implementation

The project uses an ESP32 micro controller with Wi-Fi capabilities. There is also an I2C OLED screen and a physical push button.

Currently this project has been developed to work with Arizona, California, Florida and Texas. Once a location has been chosen, the co-ordinates will be used to make an API request. This data will then be processed and compared against the database containing 15 different plants.

The recommendations will be displayed on the OLED screen.

##  Development

The concept, research, API choice, design, flowchart and wiring were all done by myself.

I had to learn how to use NASA API's and other also learnt about databases and json.

## 🧰 Technologies Used

Hardware:

ESP32 DevKit

128x64 I2C OLED

Push Button

Software/libraries needed:

C++

Arduino IDE

ArduinoJson

Adafruit GFX

Adafruit SSD1306

ESP32 WIFI AND HTTPCLIENT libraries

Data:

NASA POWER API and USDA Soil Data Access API

Testing:

Wokwi

Hardware testing

Ide

## 🚀 Current Development

This is currently a functioning prototype. I am currently working on verifying the data being returned from the APIs and improving the processing of the environmental data. I am also testing the screen display and making sure the ranges for the plants are accurate scientifically.

## 🌟 Vision

This project was designed to show how freely available earth science data can be used to create helpful farming applications. This project looks at how environmental data can be used to help farmers find suitable native plants.
## **AI Acknowledgement**

AI was used to assist with coding and debugging, while the original idea, research, API selection, system design and hardware implementation are my own.
