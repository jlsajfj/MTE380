const mapCanvas = document.getElementById("map");
const mapSize = mapCanvas.getBoundingClientRect().height;
const mapCtx = mapCanvas.getContext("2d");

var prevPos = [];
var initialized = false;
var fx = 0.0, fy = 0.0;
var cx = 0.0, cy = 0.0;

function mapInit() {
    console.log(mapCanvas);
    setInterval(drawMap, 100);
    drawMap();
}

function updateMap() {
    let px = current_data.px, py = current_data.py;
    cx = px, cy = py;
    if(!initialized) {// first run
        fx = px;
        fy = py;
        initialized = true;
        mapInit();
        return;
    }

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
    mapCtx.fillStyle = "#dc322f";
    mapCtx.beginPath();
    mapCtx.arc(cx, cy, 3, 0, 2 * Math.PI);
    mapCtx.fill();
    mapCtx.stroke();
}
