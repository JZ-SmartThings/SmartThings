/**
 *  ESP32 Contact Sensor v1.0.20190811
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
	definition (name: "ESP32 Contact Sensor", namespace: "JZ", author: "JZ") {
		capability "Contact Sensor"
		capability "Sensor"
		capability "Switch"
		command "open"
		command "close"
		command "on"
		command "off"
	}
	simulator {
		status "open": "contact:open"
		status "closed": "contact:closed"
	}
	tiles(scale: 2) {
		standardTile("contact", "device.contact", width: 6, height: 2) {
			state("closed", label:'${name}', icon:"st.contact.contact.closed", backgroundColor:"#53a7c0")
			state("open", label:'${name}', icon:"st.contact.contact.open", backgroundColor:"#FF6600")
		}
		standardTile("switch", "device.switch", width: 6, height: 2, canChangeIcon: true) {
			state "off", label: '${currentValue}', action: "switch.on", icon: "st.switches.switch.off", backgroundColor: "#ffffff"
			state "on", label: '${currentValue}', action: "switch.off", icon: "st.switches.switch.on", backgroundColor: "#79b821"
		}
		main "contact"
		details (["contact"])
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

def open() {
	log.trace "open()"
	sendEvent(name: "contact", value: "open")
}

def close() {
	log.trace "close()"
    sendEvent(name: "contact", value: "closed")
}

def on() {
	log.debug "$version on()"
	sendEvent(name: "switch", value: "on")
	sendEvent(name: "contact", value: "open")
}

def off() {
	log.debug "$version off()"
	sendEvent(name: "switch", value: "off")
    sendEvent(name: "contact", value: "closed")
}
