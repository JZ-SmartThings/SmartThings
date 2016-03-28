/**
 *  Generic HTTP Device v1.0.20160327
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

metadata {
	definition (name: "Generic HTTP Device", author: "JZ", namespace:"JZ") {
		capability "Switch"
		attribute "hubactionMode", "string"
		attribute "lastTriggered", "string"
		attribute "testTriggered", "string"
		attribute "triggerswitch", "string"
		attribute "cpuUsage", "string"
		attribute "spaceUsed", "string"
		attribute "upTime", "string"
		attribute "cpuTemp", "string"
		command "DeviceTrigger"
		command "TestTrigger"
		command "RebootNow"
		command "ResetTiles"
	}


    preferences {
//		input("DeviceButtonName", "string", title:"Button Name", description: "Please enter button name", required: false, displayDuringSetup: true)
		input("DeviceIP", "string", title:"Device IP Address", description: "Please enter your device's IP Address", required: true, displayDuringSetup: true)
		input("DevicePort", "string", title:"Device Port", description: "Please enter port 80 or your device's Port", required: true, displayDuringSetup: true)
		input("DevicePath", "string", title:"URL Path", description: "Rest of the URL, include forward slash.", displayDuringSetup: true)
		input(name: "DevicePostGet", type: "enum", title: "POST or GET", options: ["POST","GET"], required: true, displayDuringSetup: true)
		input("DeviceBodyText", "string", title:'Body Content', description: 'Type in "GateTrigger=" for PHP POST', displayDuringSetup: true)
		section() {
			input("HTTPAuth", "bool", title:"Requires User Auth?", description: "Choose if the HTTP requires basic authentication", defaultValue: false, required: true, displayDuringSetup: true)
			input("HTTPUser", "string", title:"HTTP User", description: "Enter your basic username", required: false, displayDuringSetup: true)
			input("HTTPPassword", "string", title:"HTTP Password", description: "Enter your basic password", required: false, displayDuringSetup: true)
		}
	}
    
	simulator {
	}

	tiles {
		valueTile("lastTriggered", "device.lastTriggered", width: 2, height: 1, inactiveLabel: false, decoration: "flat") {
			state("default", label: 'Last triggered:\n${currentValue}', backgroundColor:"#ffffff")
		}
		standardTile("DeviceTrigger", "device.triggerswitch", width: 1, height: 1, canChangeIcon: true, canChangeBackground: true, decoration: "flat") {
			state "default", label:'GATE' , action: "DeviceTrigger", icon: "st.Outdoor.outdoor22", backgroundColor:"#53a7c0", nextState: "triggerrunning"
            state "triggerrunning", label: 'OPENING', action: "ResetTiles", icon: "st.Outdoor.outdoor22", backgroundColor: "#FF6600", nextState: "default"
		}
		valueTile("testTriggered", "device.testTriggered", width: 2, height: 1, inactiveLabel: false, decoration: "flat") {
			state("default", label: 'Test triggered:\n${currentValue}', backgroundColor:"#ffffff")
		}
		standardTile("TestTrigger", "device.testswitch", width: 1, height: 1, decoration: "flat") {
			state "default", label:'TEST', action: "TestTrigger", icon: "st.Office.office13", backgroundColor:"#53a7c0", nextState: "testrunning"
			state "testrunning", label: 'TESTING', action: "ResetTiles", icon: "st.Office.office13", backgroundColor: "#FF6600", nextState: "default"
		}
		valueTile("cpuUsage", "device.cpuUsage", width: 1, height: 1, inactiveLabel: false, decoration: "flat") {
			state("default", label: '${currentValue}', backgroundColor:"#ffffff")
			}
		valueTile("spaceUsed", "device.spaceUsed", width: 1, height: 1, inactiveLabel: false, decoration: "flat") {
			state("default", label: '${currentValue}', backgroundColor:"#ffffff")
		}
		valueTile("upTime", "device.upTime", width: 1, height: 1, inactiveLabel: false, decoration: "flat") {
			state("default", label: '${currentValue}', backgroundColor:"#ffffff")
		}
		valueTile("cpuTemp", "device.cpuTemp", width: 1, height: 1, inactiveLabel: false, decoration: "flat") {
			state("default", label: '${currentValue}', backgroundColor:"#ffffff")
		}
		standardTile("RebootNow", "device.switch", width: 1, height: 1, decoration: "flat") {
			state "default", label:'REBOOT' , action: "RebootNow", icon: "st.Seasonal Winter.seasonal-winter-014", backgroundColor:"#ff0000"
		}
//		standardTile("refresh", "device.refresh", inactiveLabel: false, decoration: "flat", width: 1, height: 1) {
//			state "default", action:"ResetTiles", icon:"st.secondary.refresh"
//		}
		main "DeviceTrigger"
		details(["lastTriggered", "DeviceTrigger", "testTriggered", "TestTrigger", "cpuUsage", "spaceUsed", "upTime", "cpuTemp", "RebootNow"])
	}
}

def DeviceTrigger() {
	log.debug "Gate Triggered!!!"
	//sendEvent(name: "DeviceTrigger", value: "test", unit: "")
	runCmd(DeviceBodyText)
}
def TestTrigger() {
	log.debug "Test Triggered!!!"
	runCmd('Test=')
}
def RebootNow() {
	log.debug "Reboot Triggered!!!"
	runCmd('RebootNow=')
}
def ResetTiles() {
	//RETURN BUTTONS TO CORRECT STATE
	sendEvent(name: "testswitch", value: "default", isStateChange: true)
	sendEvent(name: "triggerswitch", value: "default", isStateChange: true)
	log.debug "Resetting tiles."
}
def runCmd(String varCommand) {
	def host = DeviceIP 
	def hosthex = convertIPtoHex(host).toUpperCase()
	def porthex = convertPortToHex(DevicePort).toUpperCase()
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
	headers.put("HOST", "$host:$DevicePort")
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
	//log.debug hubAction
	hubAction
	}
	catch (Exception e) {
		log.debug "Hit Exception $e on $hubAction"
	}   

}


def parse(String description) {
//    log.debug "Parsing '${description}'"

	def whichTile = ''
	def map = [:]
	def retResult = []
	def descMap = parseDescriptionAsMap(description)
	//Image
	if (descMap["body"]) {
		//putImageInS3(descMap)
		//log.debug descMap["body"]
		def bodyReturned = new String(descMap["body"].decodeBase64())
		log.debug bodyReturned
		if (bodyReturned.contains(DeviceBodyText + 'Success')) {
			log.debug "FOUND SUCCESS!!!"
			def timeString = new Date().format("yyyy-MM-dd h:mm:ss a", location.timeZone)
			log.debug timeString
			//sendEvent(name: "GateTriggered", value: timeString as String, unit: "")
			sendEvent(name: "lastTriggered", value: timeString as String, unit: "")
			whichTile = 'main'
			//lastTriggered=timeString as String
		}
		if (bodyReturned.contains('GateTrigger=Failed : Authentication Required!')) {
			sendEvent(name: "lastTriggered", value: "Use Authentication Credentials", unit: "")
			whichTile = 'main'
		}
		if (bodyReturned.contains('Test=Failed : Authentication Required!')) {
			sendEvent(name: "testTriggered", value: "Use Authentication Credentials", unit: "")
			whichTile = 'test'
		}
		if (bodyReturned.contains('Test=Success')) {
			def timeString = new Date().format("yyyy-MM-dd h:mm:ss a", location.timeZone)
			sendEvent(name: "testTriggered", value: timeString as String, unit: "")
			whichTile = 'test'
		}
		if (bodyReturned.contains('CPU=')) {
			def cpuData=''
			def data=bodyReturned.eachLine { line ->
				if (line.contains('CPU=')) {
					cpuData=line
				}
			}
			//log.debug cpuData
			sendEvent(name: "cpuUsage", value: cpuData.toString().replace("=","\n"), unit: "")
		}
		if (bodyReturned.contains('Space Used=')) {
			def spaceUsed=''
			def data=bodyReturned.eachLine { line ->
				if (line.contains('Space Used=')) {
					spaceUsed=line
				}
			}
			sendEvent(name: "spaceUsed", value: spaceUsed.toString().replace("=","\n"), unit: "")
		}
		if (bodyReturned.contains('UpTime=')) {
            def upTime=''
            def data=bodyReturned.eachLine { line ->
            	if (line.contains('UpTime=')) {
                	upTime=line
                }
            }
			sendEvent(name: "upTime", value: upTime.toString().replace("=","\n"), unit: "")
		}
		if (bodyReturned.contains('CPU Temp=')) {
			def cpuTemp=''
			def data=bodyReturned.eachLine { line ->
				if (line.contains('CPU Temp=')) {
					line=line.replace("\n","")
					if (cpuTemp.length() == 0) {
						cpuTemp=line.replace("CPU Temp=","")
					} else {
						cpuTemp=cpuTemp+' = '+line.replace("CPU Temp=","")
					}
					//log.debug cpuTemp
				}
			}
			sendEvent(name: "cpuTemp", value: "CPU Temp\n"+cpuTemp.toString(), unit: "")
		}
	}


//	def result = createEvent(name: "testswitch", value: "default", isStateChange: true)
//	log.debug "testswitch returned ${result?.descriptionText}"
//	return result

	//RETURN BUTTONS TO CORRECT STATE
	log.debug 'whichtile: $whichtile'
	if (whichTile.contains('test')) {
		def result = createEvent(name: "testswitch", value: "default", isStateChange: true)
		log.debug "testswitch returned ${result?.descriptionText}"
		return result
	} else if (whichTile.contains('main')) {
		def result = createEvent(name: "triggerswitch", value: "default", isStateChange: true)
		log.debug "triggerswitch returned ${result?.descriptionText}"
		return result
	} else {
		def result = createEvent(name: "testswitch", value: "default", isStateChange: true)
		log.debug "testswitch returned ${result?.descriptionText}"
		return result
	}
/**
*/
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
