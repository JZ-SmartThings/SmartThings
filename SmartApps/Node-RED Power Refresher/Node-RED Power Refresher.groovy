/**
 *  Node-RED Power Refresher v1.0.20190128
 *  This works well with TP-Link HS110 Power Monitoring Switch. Shortest interval should not be less than 5 seconds or SmartThings hub may become unresponsive &CRASH!!!
 *  Copyright 2019 JZ
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 *  in compliance with the License. You may obtain a copy of the License at:
 *	  http://www.apache.org/licenses/LICENSE-2.0
 *  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 *  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 *  for the specific language governing permissions and limitations under the License.

*** Node-Red import:
[{"id":"81cd71eb.f90cf","type":"http in","z":"d0355279.302e48","name":"","url":"/powerrefresh","method":"get","upload":false,"swaggerDoc":"","x":110,"y":80,"wires":[["171f5f8f.646db","ac51dfc7.e21ee","c3a3b1cc.e18f58"]]},{"id":"81e0d24e.ee8b4","type":"function","z":"d0355279.302e48","name":"Remove Dupes","func":"var last_power = Number(flow.get('last_power')) || 0;\nflow.set('last_power',Number(msg.payload.power));\n\nif (last_power == Number(msg.payload.power)) {\n    return null;\n} else {\n    msg.payload=null;\n    msg.payload=Number(flow.get('last_power')) || 0;\n    return msg;\n}","outputs":1,"noerr":0,"x":400,"y":240,"wires":[["7dcd4771.e3572","66a3f2b.87eb98c"]]},{"id":"171f5f8f.646db","type":"debug","z":"d0355279.302e48","name":"","active":true,"tosidebar":true,"console":false,"tostatus":false,"complete":"payload.power","x":130,"y":120,"wires":[]},{"id":"7dcd4771.e3572","type":"debug","z":"d0355279.302e48","name":"","active":true,"tosidebar":true,"console":false,"tostatus":false,"complete":"payload","x":390,"y":160,"wires":[]},{"id":"ac51dfc7.e21ee","type":"http response","z":"d0355279.302e48","name":"","statusCode":"200","headers":{},"x":380,"y":40,"wires":[]},{"id":"66a3f2b.87eb98c","type":"mqtt out","z":"d0355279.302e48","name":"MQTT Power","topic":"smartthings/HVAC Pump Sensor/power","qos":"","retain":"","broker":"cb9f73e8.8d453","x":810,"y":120,"wires":[]},{"id":"938cf8b.741e088","type":"delay","z":"d0355279.302e48","name":"","pauseType":"rate","timeout":"5","timeoutUnits":"seconds","rate":"1","nbRateUnits":"15","rateUnits":"minute","randomFirst":"1","randomLast":"5","randomUnits":"seconds","drop":true,"x":580,"y":80,"wires":[["66a3f2b.87eb98c","90c6db12.ec0278"]]},{"id":"90c6db12.ec0278","type":"debug","z":"d0355279.302e48","name":"","active":true,"tosidebar":true,"console":false,"tostatus":false,"complete":"payload","x":810,"y":80,"wires":[]},{"id":"c3a3b1cc.e18f58","type":"function","z":"d0355279.302e48","name":"Fix Payload","func":"tmp_payload=Number(msg.payload.power) || 0;\nmsg.payload=null;\nmsg.payload=tmp_payload\nreturn msg;\n","outputs":1,"noerr":0,"x":390,"y":80,"wires":[["938cf8b.741e088","9932436a.33a088"]]},{"id":"9932436a.33a088","type":"rbe","z":"d0355279.302e48","name":"","func":"rbe","gap":"","start":"","inout":"out","property":"payload","x":550,"y":120,"wires":[["66a3f2b.87eb98c"]]},{"id":"cb9f73e8.8d453","type":"mqtt-broker","z":"","broker":"localhost","port":"1883","clientid":"","usetls":false,"compatmode":true,"keepalive":"60","cleansession":true,"birthTopic":"","birthQos":"0","birthPayload":"","willTopic":"","willQos":"0","willPayload":""}] 

 */

definition(
	name: "Node-RED Power Refresher",
	namespace: "JZ",
	author: "JZ",
	description: "Node-RED Power Refresher calls the refresh method of a single device and posts to Node-RED.",
	category: "",
	iconUrl: "https://s3.amazonaws.com/smartapp-icons/Convenience/Cat-Convenience.png",
	iconX2Url: "https://s3.amazonaws.com/smartapp-icons/Convenience/Cat-Convenience@2x.png",
	iconX3Url: "https://s3.amazonaws.com/smartapp-icons/Convenience/Cat-Convenience@2x.png")

preferences {
	section("Node-RED Power Refresher Configuration:") {
		input ("refreshdevice", "capability.refresh", title: "Device to Refresh", multiple: false, required: true)
	}
	section("Refresh Interval in Seconds. 0 or null turns refreshing off:") {
		input ("refreshfreq", "number", title: "Refresh Frequency in seconds?", multiple: false, required: true, default: 0)
	}
	section("Host:Port of Node-RED:") {
		input ("hostport", "string", title: "Host and port of Node-RED?", multiple: false, required: true, default: "192.168.0.123:1880")
	}
	section("The rest of the page URL endpoint after forward slash:") {
		input ("urlending", "string", title: "URL endpoint page?", multiple: false, required: false, default: "powerrefresh")
	}
	section("Form attribute name:") {
		input ("formattribute", "string", title: "Form attribute?", multiple: false, required: false, default: "power")
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
	if (refreshdevice) {
		subscribe(app, runApp)
	}
	if (refreshfreq > 0) {
		schedule(now() + refreshfreq*1000, refreshFunc)
	}

}

def callRefresh(evt) {
	refreshdevice.refresh()
}

def runApp(evt) {
	log.debug "Manual refresh of " + settings["refreshdevice"] + " triggered. Currently set to refresh every " + refreshfreq + " seconds."
	refreshdevice.refresh()
}

def refreshFunc() {
	refreshdevice.refresh()
	//log.debug "Auto refresh of " + settings["refreshdevice"] + " triggered. Currently set to refresh every " + refreshfreq + " seconds."
	schedule(now() + refreshfreq*1000, refreshFunc)
	
	if (state.lastvalue != String.format('%6.0f', refreshdevice*.currentValue('power')[0]).trim()) {
		state.lastvalue = String.format('%6.0f', refreshdevice*.currentValue('power')[0]).trim()
		def theAction = new physicalgraph.device.HubAction("""GET /${settings['urlending']}?${settings['formattribute']}=${(String.format('%6.0f', refreshdevice*.currentValue('power')[0]).trim())} HTTP/1.1\r\n Accept: */*\r\nHOST: ${settings['hostport']}\r\n\r\n""", physicalgraph.device.Protocol.LAN, settings['hostport'], [callback: calledBackHandler])
		sendHubCommand(theAction)
		//def theAction = new physicalgraph.device.HubAction("""GET /zoozpower?power=${(String.format('%6.0f', refreshdevice*.currentValue('power')[0]).trim())} HTTP/1.1\r\n Accept: */*\r\nHOST: 192.168.0.251:1880\r\n\r\n""", physicalgraph.device.Protocol.LAN, "192.168.0.251:1880", [callback: calledBackHandler])
	}
}
void calledBackHandler(physicalgraph.device.HubResponse hubResponse)
{
    log.debug "Reponse ${hubResponse.body}"
}