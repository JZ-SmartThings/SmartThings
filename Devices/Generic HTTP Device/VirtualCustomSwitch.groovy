/**
 *  Virtual Custom Switch v1.0.20170327
 *  Copyright 2017 JZ
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 *  in compliance with the License. You may obtain a copy of the License at:
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 *  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 *  for the specific language governing permissions and limitations under the License.
 */
metadata {
	definition (name: "Virtual Custom Switch", namespace: "JZ", author: "JZ") {
		capability "Switch"
		
		command "onPhysical"
		command "offPhysical"
	}

	tiles {
		standardTile("switch", "device.switch", width: 2, height: 2, canChangeIcon: true) {
			state "off", label: '${currentValue}', action: "switch.on", icon: "st.switches.switch.off", backgroundColor: "#ffffff"
			state "on", label: '${currentValue}', action: "switch.off", icon: "st.switches.switch.on", backgroundColor: "#79b821"
		}
		standardTile("on", "device.switch", decoration: "flat") {
			state "default", label: 'On', action: "onPhysical", backgroundColor: "#ffffff"
		}
		standardTile("off", "device.switch", decoration: "flat") {
			state "default", label: 'Off', action: "offPhysical", backgroundColor: "#ffffff"
		}
		valueTile("customTriggered", "device.customTriggered", width: 3, height: 1, decoration: "flat") {
			state("default", label: 'Custom triggered:\r\n${currentValue}', backgroundColor:"#ffffff")
		}

        main "switch"
		details(["switch","on","off","customTriggered"])
	}
}

def parse(description) {
	def eventMap
	if (description.type == null) eventMap = [name:"$description.name", value:"$description.value"]
	else eventMap = [name:"$description.name", value:"$description.value", type:"$description.type"]
	createEvent(eventMap)
}

def on() {
	log.debug "$version on()"
	sendEvent(name: "switch", value: "on")
}

def off() {
	log.debug "$version off()"
	sendEvent(name: "switch", value: "off")
}

def onPhysical() {
	log.debug "$version onPhysical()"
	sendEvent(name: "switch", value: "on", type: "physical")
}

def offPhysical() {
	log.debug "$version offPhysical()"
	sendEvent(name: "switch", value: "off", type: "physical")
}

private getVersion() {
	"PUBLISHED"
}