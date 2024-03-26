const mapCanvas = document.getElementById("map");
const mapSize = mapCanvas.getBoundingClientRect().height;
const mapCtx = mapCanvas.getContext("2d");

var prevPos = [];
var initialized = false;
var fx = 0.0, fy = 0.0;
var cx = 0.0, cy = 0.0;
var rx = 0.0, ry = 0.0;
var runner;

function mapInit() {
    runner = setInterval(drawMap, 100);
    drawMap();
}

function resetMap() {
    clearInterval(runner);
    initialized = false;
    prevPos = [];
}

function updateMap() {
    let px = current_data.px, py = current_data.py;
    rx = current_data.rx, ry = current_data.ry;
    if(!initialized) {// first run
        fx = px;
        fy = py;
        initialized = true;
        mapInit();
        cx = px, cy = py;
        return;
    }

    if(px == cx && py == cy) return;

    cx = px, cy = py;
    prevPos.push([px, py]);
}

function drawMap() {
    mapCtx.clearRect(0, 0, 600, 600);
    mapCtx.lineWidth = 1;
    mapCtx.strokeStyle = "#839496";

    for(var i = 0; i < 5; i ++){
        mapCtx.beginPath();
        mapCtx.moveTo(100 + i * 100, 0);
        mapCtx.lineTo(100 + i * 100, 600);
        mapCtx.stroke();
        mapCtx.beginPath();
        mapCtx.moveTo(0, 100 + i * 100);
        mapCtx.lineTo(600, 100 + i * 100);
        mapCtx.stroke();
    }

    mapCtx.strokeStyle = "#dc322f";

    mapCtx.strokeStyle = "#b58900";
    mapCtx.lineWidth = 3;
    mapCtx.beginPath();
    mapCtx.moveTo(fx, fy);
    prevPos.forEach( pos => mapCtx.lineTo(pos[0], pos[1]) );
    mapCtx.stroke();

    mapCtx.strokeStyle = "#cb4b16";
    mapCtx.beginPath();
    mapCtx.fillStyle = "#002b36";
    mapCtx.beginPath();
    mapCtx.arc(cx, cy, 4, 0, 2 * Math.PI);
    mapCtx.fill();
    mapCtx.stroke();

    mapCtx.strokeStyle = "#d33682";
    mapCtx.beginPath();
    mapCtx.fillStyle = "#002b36";
    mapCtx.beginPath();
    mapCtx.arc(rx, ry, 4, 0, 2 * Math.PI);
    mapCtx.fill();
    mapCtx.stroke();
}
