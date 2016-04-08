/**
 *  Generic HTTP Device v1.0.20160408
 *
 *  Source code can be found here: https://github.com/JZ-SmartThings/SmartThings/blob/master/Devices/Generic%20HTTP%20Device/GenericHTTPDevice.groovy
 *
 *  Copyright 2016 JZ
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 *  in compliance with the License. You may obtain a copy of the License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 *  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 *  for the specific language governing permissions and limitations under the License.
 *
 */

import groovy.json.JsonSlurper

metadata {
	definition (name: "Generic HTTP Device", author: "JZ", namespace:"JZ") {
		capability "Switch"
		capability "Polling"
		capability "Refresh"
		attribute "hubactionMode", "string"
		attribute "lastTriggered", "string"
		attribute "testTriggered", "string"
		attribute "customTriggered", "string"
		attribute "cpuUsage", "string"
		attribute "spaceUsed", "string"
		attribute "upTime", "string"
		attribute "cpuTemp", "string"
		attribute "freeMem", "string"
		command "DeviceTrigger"
		command "TestTrigger"
		command "CustomTrigger"
		command "RebootNow"
		command "ResetTiles"
		command "ClearTiles"
	}

	preferences {
//		input("DeviceButtonName", "string", title:"Button Name", description: "Please enter button name", required: false, displayDuringSetup: true)
		input("DeviceIP", "string", title:"Device IP Address", description: "Please enter your device's IP Address", required: true, displayDuringSetup: true)
		input("DevicePort", "string", title:"Device Port", description: "Empty assumes port 80.", required: false, displayDuringSetup: true)
		input("DevicePath", "string", title:"URL Path", description: "Rest of the URL, include forward slash.", displayDuringSetup: true)
		input(name: "DevicePostGet", type: "enum", title: "POST or GET", options: ["POST","GET"], required: true, displayDuringSetup: true)
		input("DeviceBodyText", "string", title:'Body Content', description: 'Empty assumes "GateTrigger="', required: false, displayDuringSetup: false)
		input("UseJSON", "bool", title:"Use JSON instead of HTML?", description: "Use JSON instead of HTML?", defaultValue: false, required: false, displayDuringSetup: true)
		section() {
			input("HTTPAuth", "bool", title:"Requires User Auth?", description: "Choose if the HTTP requires basic authentication", defaultValue: false, required: true, displayDuringSetup: true)
			input("HTTPUser", "string", title:"HTTP User", description: "Enter your basic username", required: false, displayDuringSetup: true)
			input("HTTPPassword", "string", title:"HTTP Password", description: "Enter your basic password", required: false, displayDuringSetup: true)
		}
	}
	
	simulator {
	}

	tiles(scale: 2) {
		valueTile("lastTriggered", "device.lastTriggered", width: 5, height: 1, decoration: "flat") {
			state("default", label: 'Last triggered:\n${currentValue}', backgroundColor:"#ffffff")
		}
		standardTile("DeviceTrigger", "device.triggerswitch", width: 1, height: 1, canChangeIcon: true, canChangeBackground: true, decoration: "flat") {
			state "default", label:'GATE' , action: "on", icon: "st.Outdoor.outdoor22", backgroundColor:"#53a7c0", nextState: "triggerrunning"
			state "triggerrunning", label: 'OPENING', action: "ResetTiles", icon: "st.Outdoor.outdoor22", backgroundColor: "#FF6600", nextState: "default"
		}
		valueTile("customTriggered", "device.customTriggered", width: 5, height: 1, decoration: "flat") {
			state("default", label: 'Custom triggered:\n${currentValue}', backgroundColor:"#ffffff")
		}
		standardTile("CustomTrigger", "device.customswitch", width: 1, height: 1, decoration: "flat") {
			state "default", label:'CUSTOM', action: "off", icon: "st.Lighting.light13", backgroundColor:"#53a7c0", nextState: "customrunning"
			state "customrunning", label: 'RUNNING', action: "ResetTiles", icon: "st.Lighting.light13", backgroundColor: "#FF6600", nextState: "default"
		}
		valueTile("testTriggered", "device.testTriggered", width: 5, height: 1, decoration: "flat") {
			state("default", label: 'Test triggered:\n${currentValue}', backgroundColor:"#ffffff")
		}
		standardTile("TestTrigger", "device.testswitch", width: 1, height: 1, decoration: "flat") {
			state "default", label:'TEST', action: "poll", icon: "st.Office.office13", backgroundColor:"#53a7c0", nextState: "testrunning"
			state "testrunning", label: 'TESTING', action: "ResetTiles", icon: "st.Office.office13", backgroundColor: "#FF6600", nextState: "default"
		}
		valueTile("cpuUsage", "device.cpuUsage", width: 2, height: 2) {
			state("default", label: 'CPU\n ${currentValue}%',
				backgroundColors:[
					[value: 0, color: "#00cc33"],
					[value: 10, color: "#99ff33"],
					[value: 30, color: "#ffcc99"],
					[value: 55, color: "#ff6600"],
					[value: 90, color: "#ff0000"]
				]
			)
		}
		valueTile("cpuTemp", "device.cpuTemp", width: 2, height: 2) {
			state("default", label: 'CPU Temp ${currentValue}',
				backgroundColors:[
					[value: 50, color: "#00cc33"],
					[value: 60, color: "#99ff33"],
					[value: 67, color: "#ff6600"],
					[value: 75, color: "#ff0000"]
				]
			)
		}
		valueTile("spaceUsed", "device.spaceUsed", width: 2, height: 2) {
			state("default", label: 'Space Used\n ${currentValue}%',
				backgroundColors:[
					[value: 50, color: "#00cc33"],
					[value: 75, color: "#ffcc66"],
					[value: 85, color: "#ff6600"],
					[value: 95, color: "#ff0000"]
				]
			)
		}
		valueTile("upTime", "device.upTime", width: 2, height: 2, decoration: "flat") {
			state("default", label: 'UpTime\n ${currentValue}', backgroundColor:"#ffffff")
		}
		valueTile("freeMem", "device.freeMem", width: 2, height: 2, decoration: "flat") {
			state("default", label: 'Free Mem\n ${currentValue}', backgroundColor:"#ffffff")
		}
		standardTile("clearTiles", "device.clear", width: 2, height: 2, decoration: "flat") {
			state "default", label:'Clear Tiles', action:"ClearTiles", icon:"st.Bath.bath9"
		}
		standardTile("RebootNow", "device.rebootnow", width: 1, height: 1, decoration: "flat") {
			state "default", label:'REBOOT' , action: "RebootNow", icon: "st.Seasonal Winter.seasonal-winter-014", backgroundColor:"#ff0000", nextState: "rebooting"
			state "rebooting", label: 'REBOOTING', action: "ResetTiles", icon: "st.Office.office13", backgroundColor: "#FF6600", nextState: "default"
		}
		main "DeviceTrigger"
		details(["lastTriggered", "DeviceTrigger", "customTriggered", "CustomTrigger", "testTriggered", "TestTrigger", "cpuUsage", "cpuTemp", "upTime", "spaceUsed", "freeMem", "clearTiles", "RebootNow"])
	}
}

def refresh() {
    poll()
}
def poll() {
	if (UseJSON==true) {
		log.debug "Test Triggered!!!"
		runCmd('Test=&UseJSON=')
	} else {
		log.debug "Test JSON Triggered!!!"
		runCmd('Test=')
	}
}
def on() {
	def LocalDeviceBodyText = ''
	if (DeviceBodyText==null) { LocalDeviceBodyText = "GateTrigger=" } else { LocalDeviceBodyText = DeviceBodyText }

	if (UseJSON==true) {
		log.debug LocalDeviceBodyText + " Triggered!!!"
		runCmd(LocalDeviceBodyText + '&UseJSON=')
	} else {
		log.debug LocalDeviceBodyText + " JSON Triggered!!!"
		runCmd(LocalDeviceBodyText)
	}
}
def off() {
	if (UseJSON==true) {
		log.debug "Custom Triggered!!!"
		runCmd('CustomTrigger=&UseJSON=')
	} else {
		log.debug "Custom JSON Triggered!!!"
		runCmd('CustomTrigger=')
	}
}
def RebootNow() {
	log.debug "Reboot Triggered!!!"
	runCmd('RebootNow=')
}
def ClearTiles() {
	sendEvent(name: "lastTriggered", value: "", unit: "")
	sendEvent(name: "customTriggered", value: "", unit: "")
	sendEvent(name: "testTriggered", value: "", unit: "")
	sendEvent(name: "cpuUsage", value: "", unit: "")
	sendEvent(name: "cpuTemp", value: "", unit: "")
	sendEvent(name: "spaceUsed", value: "", unit: "")
	sendEvent(name: "upTime", value: "", unit: "")
	sendEvent(name: "freeMem", value: "", unit: "")
}
def ResetTiles() {
	//RETURN BUTTONS TO CORRECT STATE
	sendEvent(name: "triggerswitch", value: "default", isStateChange: true)
	sendEvent(name: "customswitch", value: "default", isStateChange: true)
	sendEvent(name: "testswitch", value: "default", isStateChange: true)
	sendEvent(name: "rebootnow", value: "default", isStateChange: true)
	log.debug "Resetting tiles."
}

def runCmd(String varCommand) {
	def host = DeviceIP 
	def hosthex = convertIPtoHex(host).toUpperCase()
	def LocalDevicePort = ''
	if (DevicePort==null) { LocalDevicePort = "80" } else { LocalDevicePort = DevicePort }
	def porthex = convertPortToHex(LocalDevicePort).toUpperCase()
	device.deviceNetworkId = "$hosthex:$porthex" 
	def userpassascii = "${HTTPUser}:${HTTPPassword}"
	def userpass = "Basic " + userpassascii.encodeAsBase64().toString()

	log.debug "The device id configured is: $device.deviceNetworkId"

	def path = DevicePath
	log.debug "path is: $path"
	log.debug "Uses which method: $DevicePostGet"
	def body = varCommand 
	log.debug "body is: $body"

	def headers = [:] 
	headers.put("HOST", "$host:$LocalDevicePort")
	headers.put("Content-Type", "application/x-www-form-urlencoded")
	if (HTTPAuth) {
		headers.put("Authorization", userpass)
	}
	log.debug "The Header is $headers"
	def method = "POST"
	try {
		if (DevicePostGet.toUpperCase() == "GET") {
			method = "GET"
			}
		}
	catch (Exception e) {
		settings.DevicePostGet = "POST"
		log.debug e
		log.debug "You must not have set the preference for the DevicePOSTGET option"
	}
	log.debug "The method is $method"
	try {
		def hubAction = new physicalgraph.device.HubAction(
			method: method,
			path: path,
			body: body,
			headers: headers
			)
		hubAction.options = [outputMsgToS3:false]
		log.debug hubAction
		hubAction
	}
	catch (Exception e) {
		log.debug "Hit Exception $e on $hubAction"
	}
}

def parse(String description) {
//	log.debug "Parsing '${description}'"
	def whichTile = ''
	def map = [:]
	def retResult = []
	def descMap = parseDescriptionAsMap(description)
	def jsonlist = [:]
	def bodyReturned = new String(descMap["body"].decodeBase64())
	def headersReturned = new String(descMap["headers"].decodeBase64())
	log.debug "BODY---" + bodyReturned
	//log.debug "HEADERS---" + headersReturned
	def LocalDeviceBodyText = ''
	if (DeviceBodyText==null) { LocalDeviceBodyText = "GateTrigger=" } else { LocalDeviceBodyText = DeviceBodyText }

	if (descMap["body"] && headersReturned.contains("application/json")) {
		def body = new String(descMap["body"].decodeBase64())
		def slurper = new JsonSlurper()
		jsonlist = slurper.parseText(body)
		//log.debug "JSONLIST---" + jsonlist."CPU"
		jsonlist.put ("Date", new Date().format("yyyy-MM-dd h:mm:ss a", location.timeZone))
	} else if (descMap["body"] && headersReturned.contains("text/html")) {
		jsonlist.put ("Date", new Date().format("yyyy-MM-dd h:mm:ss a", location.timeZone))
		def data=bodyReturned.eachLine { line ->
			if (line.contains('CPU=')) { jsonlist.put ("CPU", line.replace("CPU=","")) }
			if (line.contains('Space Used=')) { jsonlist.put ("Space Used", line.replace("Space Used=","")) }
			if (line.contains('UpTime=')) { jsonlist.put ("UpTime", line.replace("UpTime=","")) }
			if (line.contains('CPU Temp=')) { jsonlist.put ("CPU Temp",line.replace("CPU Temp=","")) }
			if (line.contains('Free Mem=')) { jsonlist.put ("Free Mem",line.replace("Free Mem=",""))  }
			if (line.contains('CustomTrigger=Success')) { jsonlist.put ("CustomTrigger", "Success") }
			if (line.contains(LocalDeviceBodyText + 'Success')) { jsonlist.put (LocalDeviceBodyText.replace("=",""), "Success") }
			if (line.contains(LocalDeviceBodyText + 'Failed : Authentication Required!')) { jsonlist.put (LocalDeviceBodyText.replace("=",""), "Authentication Required!") }
			if (line.contains('CustomTrigger=Success')) { jsonlist.put ("CustomTrigger", "Success") }
			if (line.contains('CustomTrigger=Failed : Authentication Required!')) { jsonlist.put ("CustomTrigger", "Authentication Required!") }
			if (line.contains('Test=Success')) { jsonlist.put ("Test", "Success") }
			if (line.contains('Test=Failed : Authentication Required!')) { jsonlist.put ("Test", "Authentication Required!") }
			if (line.contains('RebootNow=Success')) { jsonlist.put ("RebootNow", "Success") }
			if (line.contains('RebootNow=Failed : Authentication Required!')) { jsonlist.put ("RebootNow", "Authentication Required!") }
		}
	}

	if (descMap["body"] && (headersReturned.contains("application/json") || headersReturned.contains("text/html"))) {
		//putImageInS3(descMap)
		if (jsonlist."Test"=="Authentication Required!") {
			sendEvent(name: "testTriggered", value: "Use Authentication Credentials", unit: "")
			whichTile = 'test'
		}
		if (jsonlist."Test"=="Success") {
			sendEvent(name: "testTriggered", value: jsonlist."Date", unit: "")
			whichTile = 'test'
		}
		if (jsonlist."CustomTrigger"=="Authentication Required!") {
			sendEvent(name: "customTriggered", value: "Use Authentication Credentials", unit: "")
			whichTile = 'custom'
		}
		if (jsonlist."CustomTrigger"=="Success") {
			sendEvent(name: "customTriggered", value: jsonlist."Date", unit: "")
			whichTile = 'custom'
		}
		if (jsonlist."${LocalDeviceBodyText.replace("=","")}"=="Success") {
			log.debug LocalDeviceBodyText.replace("=","") + " --- RETURNED SUCCESS!!!"
			sendEvent(name: "lastTriggered", value: jsonlist."Date", unit: "")
			whichTile = 'main'
		}
		if (jsonlist."${LocalDeviceBodyText.replace("=","")}"=="Authentication Required!") {
			sendEvent(name: "lastTriggered", value: "Use Authentication Credentials", unit: "")
			whichTile = 'main'
		}
		if (jsonlist."CPU") {
			sendEvent(name: "cpuUsage", value: jsonlist."CPU".replace("=","\n").replace("%",""), unit: "")
		}
		if (jsonlist."Space Used") {
			sendEvent(name: "spaceUsed", value: jsonlist."Space Used".replace("=","\n").replace("%",""), unit: "")
		}
		if (jsonlist."UpTime") {
			sendEvent(name: "upTime", value: jsonlist."UpTime".replace("=","\n"), unit: "")
		}
		if (jsonlist."CPU Temp") {
			sendEvent(name: "cpuTemp", value: jsonlist."CPU Temp".replace("=","\n").replace("\'","°").replace("C ","C="), unit: "")
		}
		if (jsonlist."Free Mem") {
			sendEvent(name: "freeMem", value: jsonlist."Free Mem".replace("=","\n"), unit: "")
		}
		if (jsonlist."RebootNow") {
			whichTile = 'RebootNow'
		}
	}

	log.debug jsonlist

//	def result = createEvent(name: "testswitch", value: "default", isStateChange: true)
//	log.debug "testswitch returned ${result?.descriptionText}"
//	return result

	//RETURN BUTTONS TO CORRECT STATE
	//log.debug 'whichTile: ' + whichTile
    switch (whichTile) {
        case 'test':
			def result = createEvent(name: "testswitch", value: "default", isStateChange: true)
			//log.debug "testswitch returned ${result?.descriptionText}"
			return result
        case 'custom':
			def result = createEvent(name: "customswitch", value: "default", isStateChange: true)
			return result
        case 'main':
			def result = createEvent(name: "triggerswitch", value: "default", isStateChange: true)
			return result
        case 'RebootNow':
			def result = createEvent(name: "rebootnow", value: "default", isStateChange: true)
			return result
        default:
			def result = createEvent(name: "testswitch", value: "default", isStateChange: true)
			//log.debug "testswitch returned ${result?.descriptionText}"
			return result
    }
}

def parseDescriptionAsMap(description) {
	description.split(",").inject([:]) { map, param ->
	def nameAndValue = param.split(":")
	map += [(nameAndValue[0].trim()):nameAndValue[1].trim()]
	}
}
private String convertIPtoHex(ipAddress) { 
	String hex = ipAddress.tokenize( '.' ).collect {  String.format( '%02x', it.toInteger() ) }.join()
	//log.debug "IP address entered is $ipAddress and the converted hex code is $hex"
	return hex
}
private String convertPortToHex(port) {
	String hexport = port.toString().format( '%04x', port.toInteger() )
	//log.debug hexport
	return hexport
}
private Integer convertHexToInt(hex) {
	Integer.parseInt(hex,16)
}
private String convertHexToIP(hex) {
	//log.debug("Convert hex to ip: $hex") 
	[convertHexToInt(hex[0..1]),convertHexToInt(hex[2..3]),convertHexToInt(hex[4..5]),convertHexToInt(hex[6..7])].join(".")
}
private getHostAddress() {
	def parts = device.deviceNetworkId.split(":")
	//log.debug device.deviceNetworkId
	def ip = convertHexToIP(parts[0])
	def port = convertHexToInt(parts[1])
	return ip + ":" + port
}
