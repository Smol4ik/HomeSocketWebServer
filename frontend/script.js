const btnLedOn = document.getElementById('on-led')
const btnLedOff = document.getElementById('off-led')
const temperaturaValue = document.getElementById('temperatura-id')
const humidityValue = document.getElementById('humidity-id')

const socket = new WebSocket("ws://0.0.0.0:8765");

let userList = []

socket.onopen = () => {
    console.log("WebSocket connected");
        const name = {
            type: "name",
            name: "frontend"
        }
        socket.send(JSON.stringify(name))
    }

socket.onmessage = (event) => {
    try {
        dataFromServer = JSON.parse(event.data) 
        console.log("Message from server:", dataFromServer);
        if (dataFromServer["type"] == "data_information")
            if ("temperature" in dataFromServer && "humidity" in dataFromServer) {
                if (dataFromServer["temperature"] != null && dataFromServer["humidity"] != null) {
                    temperaturaValue.innerText = `${dataFromServer["temperature"]} °C`
                    humidityValue.innerText = `${dataFromServer["humidity"]} %`
                }
                else if (dataFromServer["temperature"] != null && dataFromServer["humidity"] == null) {
                    temperaturaValue.innerText = `${dataFromServer["temperature"]} °C`
                    humidityValue.innerText = ""
                }
                else if (dataFromServer["temperature"] == null && dataFromServer["humidity"] != null) {
                    temperaturaValue.innerText = ""
                    humidityValue.innerText = `${dataFromServer["humidity"]} %`
                }
                else {
                    temperaturaValue.innerText = ""
                    humidityValue.innerText = ""
                }
            }
            else {
                console.log("Unknown message type")
            }
        else if (dataFromServer["type"] == "update_user_list") {
            userList = dataFromServer["active_clients"]
        }
    } catch (error) {
        console.log("Parse was fell", error)
    }
        
};

socket.onclose = () => {
    console.log("[DISCONNECTED] З'єднання закрито");
  };
  
  socket.onerror = (error) => {
    console.error("[ERROR] Помилка з'єднання:", error);
  };

function ledState(state) {
    /*if (userList.length == 0) {
        return
    }*/
    if (userList.includes("espled")) {
        let ledStateMsg = {
            "type": "data_information",
            "selected_client": "espled",
            "data": {"led_state": state}
        }
        socket.send(JSON.stringify(ledStateMsg))
    }
    else {
        console.log("Client Led is't connected")
        return
    }
}

btnLedOn.addEventListener('click', () => ledState("on"))
btnLedOff.addEventListener('click', () => ledState("off"))
