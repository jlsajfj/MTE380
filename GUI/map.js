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
    mapCtx.lineWidth = 8;
    mapCtx.lineCap = "square";
    mapCtx.beginPath();
    mapCtx.moveTo(550, 525);
    mapCtx.lineTo(550, 575);
    mapCtx.moveTo(550, 550);
    mapCtx.lineTo(100, 550);
    mapCtx.arc(100, 500, 50, Math.PI/2, Math.PI);
    mapCtx.arc(100, 100, 50, Math.PI, 0);
    mapCtx.arc(200, 100, 50, Math.PI, 0, true);
    mapCtx.arc(300, 100, 50, Math.PI, 3*Math.PI/2);
    mapCtx.arc(500, 100, 50, 3*Math.PI/2, 0);
    mapCtx.arc(500, 200, 50, 0, Math.PI/2);
    mapCtx.arc(400, 300, 50, 3*Math.PI/2, Math.PI, true);
    //mapCtx.moveTo(500, 250);
    //mapCtx.lineTo(400, 250);
    mapCtx.stroke();

    mapCtx.strokeStyle = "#b58900";
    mapCtx.lineWidth = 3;
    mapCtx.lineCap = "butt";
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
