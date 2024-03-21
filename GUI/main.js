const C_RED = '#dc322f';
const C_ORANGE = '#b58900';
const C_YELLOW = '#859900';
const C_GREEN = '#268bd2';
const C_BLUE = '#6c71c4';
const C_PURPLE = '#d33682';
const COLORS = [C_RED, C_ORANGE, C_YELLOW, C_GREEN, C_BLUE, C_PURPLE];

let charts = [];
function buildChart(name, inputs){
    let canvas = document.getElementById(name);
    let d = inputs.map((l, i) => {
        return {
            label: l,
            backgroundColor: COLORS[i % 6],
            borderColor: COLORS[i % 6],
            data:[]
        };
    });
    let config = {
            type: 'line',
            data: {
                labels: [],
                datasets: d,
            },
            options: {
                animation: false,
                scales: {
                    x: {
                        ticks: {
                            color: "#839496",
                        },
                        grid: {
                            color: "#839496",
                        },
                    },
                    y: {
                        ticks: {
                            color: "#839496",
                        },
                        grid: {
                            color: "#839496",
                        },
                    },
                },
                color: '#839496',
            },
        };
    let chart = new Chart(canvas, config);
    charts.push(chart);
}
buildChart('speChartL', ['mtl', 'msl']);
buildChart('speChartR', ['mtr', 'msr']);
buildChart('encChart', ['mel', 'mer']);

let wurl = 'ws://localhost:8000/'

function init() {
    if (window.location.hostname === "bot.joseph.ma") {
        wurl = "ws://ws.ngrok.pizza/";
    }
    document.myform.disconnectButton.disabled = true;
    doConnect();
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
      charts.forEach(chart => addData(chart, data.data.tis, data.data));
    } else if(data.code === 'CONFIG'){
      let config = JSON.stringify(data.data, undefined, 2);
      // console.log(config);
      // document.getElementById("config").textContent = JSON.stringify(data.data,undefined, 2);
      document.getElementById('config').textContent = config;
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
