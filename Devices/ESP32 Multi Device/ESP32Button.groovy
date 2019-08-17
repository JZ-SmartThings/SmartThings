/**
 *  ESP32 Button v1.0.20190811
 *  Copyright 2019 JZ
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 *  in compliance with the License. You may obtain a copy of the License at:
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 *  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 *  for the specific language governing permissions and limitations under the License.
 */
metadata {
	definition (name: "ESP32 Button", namespace: "JZ", author: "JZ") {
		capability "Actuator"
		capability "Button"
		capability "Switch"
		capability "Sensor"
		capability "Health Check"
		command "on"
		command "off"
		command "push"
	}
	tiles(scale: 2) {
		standardTile("switch", "device.switch", width: 6, height: 3, canChangeIcon: true) {
			state "off", label: '${currentValue}', action: "switch.on", icon: "st.switches.switch.off", backgroundColor: "#ffffff"
			state "on", label: '${currentValue}', action: "switch.off", icon: "st.switches.switch.on", backgroundColor:"#53a7c0"
		}
 		standardTile("button", "device.button", width: 2, height: 2, decoration: "flat") {
			state "default", label: "Push", backgroundColor: "#ffffff", action: "push"
		} 
		main "switch"
		details (["switch"])
	}
}

def parseOLD(String description) {
	def pair = description.split(":")
	createEvent(name: pair[0].trim(), value: pair[1].trim())
}

def parse(description) {
	log.debug description
	def eventMap
	if (description.type == null) eventMap = [name:"$description.name", value:"$description.value"]
	else eventMap = [name:"$description.name", value:"$description.value", type:"$description.type"]
	createEvent(eventMap)
}

def push() {
	sendEvent(name: "button", value: "pushed", data: [buttonNumber: "1"], descriptionText: "$device.displayName button 1 was pushed", isStateChange: true)
}

def on() {
	log.debug "on()"
	sendEvent(name: "switch", value: "on")
    runIn(0,push)
    runIn(1,off)
}

def off() {
	log.debug "off()"
	sendEvent(name: "switch", value: "off")
}