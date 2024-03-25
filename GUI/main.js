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
        sta.config.options.plugins.title.text = "Current State: " + new_state;
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
    writeToScreen("connected");
    document.myform.connectButton.disabled = true;
    document.myform.disconnectButton.disabled = false;
}

function onClose(evt) {
    writeToScreen("disconnected");
    document.myform.connectButton.disabled = false;
    document.myform.disconnectButton.disabled = true;
}

function onMessage(evt) {
    let data = JSON.parse(evt.data);
    // writeToScreen(JSON.stringify(data.data));
    // console.log(data);
    if(data.code === 'ACK'){
        writeToScreen('<span style="color: lightgreen">ack</span>')
    } else if(data.code === 'NACK') {
        writeToScreen('<span style="color: red">nack</span>')
    } else if(data.code === 'STREAM'){
        current_data = data.data;
        current_data.tis /= 1000;
        // console.log(current_data.sta);
        updateCharts();
        // charts.forEach(chart => addData(chart, data.data.tis, data.data));
        updateState(data.data.sta);
    } else if(data.code === 'CONFIG'){
        updateConfig(data.data);
        // console.log(Object.keys(data.data).length);
        // let config = JSON.stringify(data.data, undefined, 2);
        // console.log(config);
        // document.getElementById("config").textContent = JSON.stringify(data.data,undefined, 2);
        // document.getElementById('config').textContent = config;
    } else if(data.code === "STATE_MAP") {
        state_map = data.data;
        console.log(state_map);
    }
}

function onError(evt) {
    writeToScreen('error: ' + evt.data);
    websocket.close();
    document.myform.connectButton.disabled = false;
    document.myform.disconnectButton.disabled = true;
}

function doSend(message) {
    writeToScreen("sent: " + message);
    websocket.send(message);
}

function keySend(element) {
    if(event.key == 'Enter'){
        event.preventDefault();
        sendText();
    }
}

function writeToScreen(message) {
    document.getElementById('output').innerHTML = message
}

window.addEventListener("load", init, false);

function sendText() {
    doSend(document.myform.inputtext.value);
    document.myform.inputtext.value = '';
}

function clearText() {
    charts.forEach( chart => {
        chart.data.labels = [];
        chart.data.datasets.forEach( dataset => {
            dataset.data = [];
        });

        chart.update();
    });
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

function updateConfig(new_config) {
    let config_cnt = Object.keys(new_config).length;
    let col_cnt = [0, 0, Math.floor(config_cnt / 3)];
    let rem = config_cnt % 3;
    col_cnt[0] = col_cnt[2] + (rem > 0 ? 1 : 0);
    col_cnt[1] = col_cnt[2] + (rem > 1 ? 1 : 0);
    let conv_conf = [];

    for(var key in new_config) {
        if(new_config.hasOwnProperty(key)) {
            conv_conf.push([key, new_config[key]]);
        }
    }

    // console.log(conv_conf);
    let col_conf = [[], [], []];
    for(var i = 0; i < config_cnt; i ++) {
        if(i < col_cnt[0]) {
            col_conf[0].push(conv_conf[i]);
        } else if(i < col_cnt[0] + col_cnt[1]) {
            col_conf[1].push(conv_conf[i]);
        } else {
            col_conf[2].push(conv_conf[i]);
        }
    }

    // console.log(col_conf);
    for(var i = 0; i < 3; i ++){
        let div_conf = document.getElementById('config-' + i);
        let new_div_conf = '';
        col_conf[i].forEach(op => {
            let conf_name = op[0], conf_val = op[1];
            let conf_row = `<div id="conf-${conf_name}" class="conf-row"><tt>${conf_name}:</tt><input type="text" value="${conf_val}"><input type="button" value="update" onclick="updateConf('${conf_name}');"></div>`
            new_div_conf += conf_row;
        });
        div_conf.innerHTML = new_div_conf;
    }
}

function updateConf(conf_name) {
    let conf_row = document.getElementById('conf-' + conf_name);
    let new_value = parseFloat(conf_row.childNodes[1].value);
    doSend('set ' + conf_name + ' ' + new_value);
}
