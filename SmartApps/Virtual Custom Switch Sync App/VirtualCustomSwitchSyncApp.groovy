/**
 *  Virtual Custom Switch Sync App v1.0.20170408
 *  Copyright 2017 JZ
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 *  in compliance with the License. You may obtain a copy of the License at:
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 *  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 *  for the specific language governing permissions and limitations under the License.
 */
definition(
    name: "Virtual Custom Switch Sync App",
    namespace: "JZ",
    author: "JZ",
    description: "Synchronize a simulated/virtual switch with the Custom Switch & 2nd Sensor of the Generic HTTP Device Handler. This helps with automation of the second button & sensor.",
    category: "",
    iconUrl: "https://s3.amazonaws.com/smartapp-icons/Convenience/Cat-Convenience.png",
    iconX2Url: "https://s3.amazonaws.com/smartapp-icons/Convenience/Cat-Convenience@2x.png",
    iconX3Url: "https://s3.amazonaws.com/smartapp-icons/Convenience/Cat-Convenience@2x.png")

preferences {
	section("Choose your Generic HTTP Device:") {
		input ("httpswitch", "capability.switch", title: "HTTP Device?", multiple: false, required: true)
	}
	section("Choose your Simulated, currently unlinked Switch:") {
		input ("virtualswitch", "capability.switch", title: "Virtual Switch?", multiple: false, required: false)
	}
	section("Choose your Simulated, currently unlinked Contact Sensor:") {
		input ("virtualsensor", "capability.sensor", title: "Virtual Contact Sensor?", multiple: false, required: false)
	}
	section("Refresh/Poll Interval in Minutes. 0 or null turns refreshing off (try not to refresh too often):") {
		input ("refreshfreq", "number", title: "Refresh/Poll Frequency in minutes?", multiple: false, required: false)
	}
}

def installed() {
	log.debug "Installed with settings: ${settings}"
	initialize()
}

def updated() {
	log.debug "Updated with settings: ${settings}"
	unsubscribe()
	unschedule()
	initialize()
}

def initialize() {
	if (httpswitch) {
        subscribe(app, runApp)
        subscribe(httpswitch, "refreshTriggered", updateRefreshTiles)
    }
	if (virtualswitch) {
		subscribe(virtualswitch, "switch", virtualSwitchHandler)
        subscribe(httpswitch, "customswitch", switchOffHandler)
        subscribe(httpswitch, "customTriggered", updateCustomTriggered)
		subscribeToCommand(virtualswitch, "refresh", callRefresh)
    }
	if (virtualsensor) {
        subscribe(httpswitch, "contact2", virtualSensorHandler)
        subscribeToCommand(virtualsensor, "refresh", callRefresh)
    }
    if (refreshfreq > 0) {
        schedule(now() + refreshfreq*1000*60, httpRefresh)
    }
}

def callRefresh(evt) {
	httpswitch.refresh()
}

def updateCustomTriggered(evt) {
	sendEvent(settings["virtualswitch"], [name:"customTriggered", value:httpswitch*.currentValue("customTriggered")[0]])
}

def runApp(evt) {
	log.debug "Manual refresh of " + settings["httpswitch"] + " triggered. Currently set to refresh every " + refreshfreq + " minutes."
	httpswitch.refresh()
}
def httpRefresh() {
	httpswitch.refresh()
	log.debug "Auto refresh of " + settings["httpswitch"] + " triggered. Currently set to refresh every " + refreshfreq + " minutes."
	schedule(now() + refreshfreq*1000*60, httpRefresh)
}

def switchOffHandler(evt) {
	//log.debug "$httpswitch.name was turned " + httpswitch*.currentValue("customswitch")
	log.debug "switchOffHandler called with event: deviceId ${evt.deviceId} name:${evt.name} source:${evt.source} value:${evt.value} isStateChange: ${evt.isStateChange()} isPhysical: ${evt.isPhysical()} isDigital: ${evt.isDigital()} data: ${evt.data} device: ${evt.device}"

	// TRYING VALUE OF customswitch FROM HTTP DEVICE RATHER THAN $evt.value
   	//sendEvent(settings["virtualswitch"], [name:"switch", value:"$evt.value"])
	for (int i = 1; i<=2; i++) { runIn(i,updateVirtualSwitch) }
	sendEvent(settings["virtualswitch"], [name:"customTriggered", value:httpswitch*.currentValue("customTriggered")[0]])
}
def virtualSwitchHandler(evt) {
	log.debug "virtualSwitchHandler called with event: deviceId ${evt.deviceId} name:${evt.name} source:${evt.source} value:${evt.value} isStateChange: ${evt.isStateChange()} isPhysical: ${evt.isPhysical()} isDigital: ${evt.isDigital()} data: ${evt.data} device: ${evt.device}"
	log.trace "EPOCH diff was: " + String.valueOf(now()-httpswitch*.currentValue("customTriggeredEPOCH")[0])
	if (now()-httpswitch*.currentValue("customTriggeredEPOCH")[0] > 3000) {
		httpswitch.off()
        //for (int i = 1; i<=2; i++) { runIn(i,updateVirtualSwitch) }
        runIn(3,updateVirtualSwitch)
		sendEvent(settings["virtualswitch"], [name:"customTriggered", value:httpswitch*.currentValue("customTriggered")[0]])
	} else {
		//for (int i = 1; i<=2; i++) { runIn(i,updateVirtualSwitch) }
        runIn(3,updateVirtualSwitch)
		sendEvent(settings["virtualswitch"], [name:"customTriggered", value:httpswitch*.currentValue("customTriggered")[0]])
	}
}

def updateVirtualSwitch() {
	log.debug "updateVirtualSwitch to ${httpswitch*.currentValue('customswitch')[0]}"
	sendEvent(settings["virtualswitch"], [name:"switch", value:httpswitch*.currentValue("customswitch")[0]])
}

def virtualSensorHandler(evt) {
	log.debug "virtualSensorHandler called with event: deviceId ${evt.deviceId} name:${evt.name} source:${evt.source} value:${evt.value} isStateChange: ${evt.isStateChange()} isPhysical: ${evt.isPhysical()} isDigital: ${evt.isDigital()} data: ${evt.data} device: ${evt.device}"
   	sendEvent(settings["virtualsensor"], [name:"contact", value:"$evt.value"])
	sendEvent(settings["virtualsensor"], [name:"sensor2Triggered", value:httpswitch*.currentValue("sensor2Triggered")[0]])
}

def updateRefreshTiles(evt) {
	log.debug "Updating REFRESH tiles"
	schedule(now() + 1000, updateRefreshEvents)
}

def updateRefreshEvents() {
	if (settings["virtualswitch"]) { sendEvent(settings["virtualswitch"], [name:"refreshTriggered", value:httpswitch*.currentValue("refreshTriggered")[0]]) }
	if (settings["virtualsensor"]) { sendEvent(settings["virtualsensor"], [name:"refreshTriggered", value:httpswitch*.currentValue("refreshTriggered")[0]]) }
}
