let state = "STANDBY";
let wurl = 'ws://localhost:8000/';
let start_time = 0;
let end_time = 0;
let running = false;
let current_data = {};
let state_map = {1: "STANDBY"};

function init() {
    if (window.location.hostname === "bot.joseph.ma") {
        wurl = "ws://ws.ngrok.pizza/";
    }
    document.myform.disconnectButton.disabled = true;
    doConnect();
}

function updateState(c_new_state) {
    new_state = state_map[c_new_state];
    if(new_state !== state) {
        state = new_state;
        if(state === "UNHOOK") {
            start_time = current_data.tis;
            running = true;
        } else if(state === "TARGET_BREAK" || state === "STANDBY" || state === "AIM") {
            running = false;
        }
    }
    if(running){
        end_time = current_data.tis;
        document.getElementById("timer-text").innerText = (end_time - start_time).toFixed(3);
    }
}

function doConnect() {
    websocket = new WebSocket(wurl);
    websocket.onopen = function (evt) {
        onOpen(evt)
    };
    websocket.onclose = function (evt) {
        onClose(evt)
    };
    websocket.onmessage = function (evt) {
        onMessage(evt)
    };
    websocket.onerror = function (evt) {
        onError(evt)
    };
}

function onOpen(evt) {
    writeToScreen("connected\n");
    document.myform.connectButton.disabled = true;
    document.myform.disconnectButton.disabled = false;
}

function onClose(evt) {
    writeToScreen("disconnected\n");
    document.myform.connectButton.disabled = false;
    document.myform.disconnectButton.disabled = true;
}

function onMessage(evt) {
    let data = JSON.parse(evt.data);
    // writeToScreen(JSON.stringify(data.data));
    // console.log(data);
    if(data.code === 'STREAM'){
      current_data = data.data;
      current_data.tis /= 1000;
      // console.log(current_data.sta);
      updateCharts();
      // charts.forEach(chart => addData(chart, data.data.tis, data.data));
      updateState(data.data.sta);
    } else if(data.code === 'CONFIG'){
      let config = JSON.stringify(data.data, undefined, 2);
      // console.log(config);
      // document.getElementById("config").textContent = JSON.stringify(data.data,undefined, 2);
      document.getElementById('config').textContent = config;
    } else if(data.code === "STATE_MAP") {
        state_map = data.data;
        console.log(state_map);
    }
}

function onError(evt) {
    writeToScreen('error: ' + evt.data + '\n');
    websocket.close();
    document.myform.connectButton.disabled = false;
    document.myform.disconnectButton.disabled = true;
}

function doSend(message) {
    writeToScreen("sent: " + message + '\n');
    websocket.send(message);
}

function keySend(element) {
    if(event.key == 'Enter'){
        event.preventDefault();
        sendText();
    }
}

function writeToScreen(message) {
    document.getElementById('output').innerText = message
}

window.addEventListener("load", init, false);

function sendText() {
    doSend(document.myform.inputtext.value);
    document.myform.inputtext.value = '';
}

function clearText() {
    clearData(encChart);
}

function doDisconnect() {
    websocket.close();
}

function updateCharts() {
    charts.forEach( chart => {
        if (chart.data.labels.length >= 100) {
            chart.data.labels.shift();
        }
        chart.data.labels.push(current_data.tis);
        chart.data.datasets.forEach((dataset) => {
            if (dataset.data.length >= 100) {
                dataset.data.shift();
            }
            // console.log(dataset);
            dataset.data.push(current_data[dataset.label]);
        });
        chart.update();
    });
}

// function to update the chart
function addData(chart, label, data) {
    if (chart.data.labels.length >= 100) {
        chart.data.labels.shift();
    }
    chart.data.labels.push(label);
    chart.data.datasets.forEach((dataset) => {
        if (dataset.data.length >= 100) {
            dataset.data.shift();
        }
        // console.log(dataset);
        dataset.data.push(data[dataset.label]);
    });
    chart.update();
}

function clearData(chart) {
    chart.data.labels = [];
    chart.data.datasets.forEach(dataset => {
        dataset.data = [];
    });
}
