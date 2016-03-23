/**
 *
 */
metadata {
	definition (name: "Generic JZ Test", namespace: "JZ", author: "JZ") {
	
		command("on")
		command("off")
	}

    preferences {
    input("CameraIP", "string", title:"Camera IP Address", description: "Please enter your camera's IP Address", required: true, displayDuringSetup: true)
	input(name: "CameraPath", type: "enum", title: "Camera Brand", options: ["D-Link","Foscam","Panasonic"], required: true, displayDuringSetup: true)
	}
    
	simulator {
    
	}

    tiles {
        standardTile("motion", "device.switch", width: 1, height: 1, canChangeIcon: false) {
			state "off", label: 'Motion Off', action: "on", icon: "st.motion.motion.inactive", backgroundColor: "#ccffcc", nextState: "on"
            state "on", label: 'Motion On', action: "off", icon: "st.motion.motion.active", backgroundColor: "#EE0000", nextState: "off"            
		}
		
        main "motion"
        details([ "motion"])
    }
}

def on() {
	log.debug "Tilt Up"
}

def off() {
	log.debug "Pan Left"
}

