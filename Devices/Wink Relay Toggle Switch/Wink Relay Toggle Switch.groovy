/**
 *  Wink Relay Toggle Switch v1.0.20171222
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
	definition (name: "Wink Relay Toggle Switch", namespace: "JZ", author: "JZ") {
        capability "Switch"
        capability "Refresh"
        attribute "syncSwitch", "string"
        attribute "lastToggled", "string"
        attribute "lastToggledEPOC", "number"
	}

	tiles(scale: 2) {
		standardTile("switch", "device.switch", width: 6, height: 4, canChangeIcon: true) {
			state "off", label: '${currentValue}', action: "switch.on", icon: "st.switches.switch.off", backgroundColor: "#ffffff"
			state "on", label: '${currentValue}', action: "switch.off", icon: "st.switches.switch.on", backgroundColor: "#79b821"
		}
		valueTile("lastToggled", "device.lastToggled", width: 4, height: 2, decoration: "flat") {
			state("default", label: '${currentValue}', backgroundColor:"#ffffff")
		}
		standardTile("refresh", "device.refresh", width: 2, height: 2, decoration: "flat") {
			state "default", label:'REFRESH', action: "refresh", icon: "st.secondary.refresh-icon", backgroundColor:"#53a7c0"
		}
        main "switch"
		details(["switch","lastToggled","refresh"])
	}

	preferences {
        input "dupeInterval", "number", title: "Seconds to ignore duplicate calls. Prevents double firing when late external calls like IFTTT/MQTT run. Important for momentary/toggle switches when externally integrated.", description: "Range: 0-75. Default: 10 sec.", range: "0..75", defaultValue: "10", displayDuringSetup: false
    }
}

def refresh() {
	log.debug "refresh()"
    def syncSwitch = device.currentState("syncSwitch").getValue()
	log.debug "$syncSwitch"
}

def parse(description) {
	def eventMap
	if (description.type == null) eventMap = [name:"$description.name", value:"$description.value"]
	else eventMap = [name:"$description.name", value:"$description.value", type:"$description.type"]
	createEvent(eventMap)
}

def on() {
	log.debug "on()"
    toggleSwitch()
}

def off() {
	log.debug "off()"
    toggleSwitch()
}

def toggleSwitch() {
	if (device.currentValue("lastToggledEPOC")==null || device.currentValue("lastToggledEPOC")==0) {
    	sendEvent(name: "lastToggledEPOC", value: 1, isStateChange: true)
    }

	if (now()-device.currentValue("lastToggledEPOC") > 0) {
        def currstate = "off"
        if (device.currentState("syncSwitch")!=null) {
        	currstate=device.currentState("syncSwitch").getValue()
        }
        
        def currdate = new Date().format("yyyy-MM-dd h:mm:ss a", location.timeZone)
        if (currstate == "on") {
            sendEvent(name: "switch", value: "off", isStateChange: true, display: false)
            sendEvent(name: "syncSwitch", value: "off", isStateChange: true, display: false)
            currdate = "OFF @ " + currdate
            sendEvent(name: "lastToggled", value: currdate)
            log.debug "Toggled: off"
            updateEPOC("Updated EPOC from ON method")
            }
        else if (currstate == "off") {
            sendEvent(name: "switch", value: "on", isStateChange: true, display: false)
            sendEvent(name: "syncSwitch", value: "on", isStateChange: true, display: false)
            currdate = "ON @ " + currdate
            sendEvent(name: "lastToggled", value: currdate)
            log.debug "Toggled: on"
            updateEPOC("Updated EPOC from OFF method")
        }
	} else {
		log.debug "Not triggered due to EPOC difference."
	}
}

def updateEPOC(String caller) {
	def dupeThreshold = 10000
	if (dupeInterval != null) {
    	dupeThreshold = dupeInterval*1000
    }

	log.debug "EPOC before update: " + device.currentValue("lastToggledEPOC")
	sendEvent(name: "lastToggledEPOC", value: now()+dupeThreshold, isStateChange: true)
	log.debug "Updated EPOC ${caller} to: " + now()+dupeThreshold
}
