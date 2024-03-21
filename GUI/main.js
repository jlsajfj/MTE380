const C_YELLOW = 'rgb(250, 237, 203)';
const C_GREEN = 'rgb(201, 228, 222)';
const C_BLUE = 'rgb(198, 222, 241)';
const C_PURPLE = 'rgb(219, 205, 240)';
const C_RED = 'rgb(242, 198, 222)';
const C_ORANGE = 'rgb(247, 217, 198)';

const speChart = new Chart(
        document.getElementById('speChart'),
        {
            type: 'line',
            data: {
                labels: [],
                datasets: [
                    {
                        label: 'msl',
                        backgroundColor: C_RED,
                        borderColor: C_RED,
                        data: [],
                    },
                    {
                        label: 'msr',
                        backgroundColor: C_ORANGE,
                        borderColor: C_ORANGE,
                        data: [],
                    },
                    {
                        label: 'mtl',
                        backgroundColor: C_YELLOW,
                        borderColor: C_YELLOW,
                        data: [],
                    },
                    {
                        label: 'mtr',
                        backgroundColor: C_GREEN,
                        borderColor: C_GREEN,
                        data: [],
                    },
                ]
            },
            options: {
                animation: false
            }
        });
const encChart = new Chart(
        document.getElementById('encChart'),
        {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                        label: 'mel',
                        backgroundColor: 'rgb(255, 99, 132)',
                        borderColor: 'rgb(255, 99, 132)',
                        data: [],
                    }, {
                        label: 'mer',
                        backgroundColor: 'rgb(132, 255, 99)',
                        borderColor: 'rgb(132, 255, 99)',
                        data: [],
                    }
                ]
            },
            options: {
                animation: false
            }
        });

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
    setInterval(() => doSend('get'), 1000);
    doSend('get');
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
      addData(encChart, data.data.tis, data.data);
      addData(speChart, data.data.tis, data.data);
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
        document.myform.inputtext.value = '';
    }
}

function writeToScreen(message) {
    document.getElementById('output').innerText = message
}

window.addEventListener("load", init, false);

function sendText() {
    doSend(document.myform.inputtext.value);
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
