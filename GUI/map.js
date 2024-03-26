const mapCanvas = document.getElementById("map");
const mapSize = mapCanvas.getBoundingClientRect().height;
const mapCtx = mapCanvas.getContext("2d");

var prevPos = [];
var initialized = false;
var fx = 0.0, fy = 0.0;
var cx = 0.0, cy = 0.0;
var rx = 0.0, ry = 0.0;
var runner;
const path = [
    [550, 550],
    [100, 550],
    [50, 500],
    [50, 100],
    [100, 50],
    [150, 100],
    [200, 150],
    [250, 100],
    [300, 50],
    [500, 50],
    [550, 100],
    [550, 200],
    [500, 250],
    [400, 250],
    [350, 300],
    [350, 350],
];

var pathLength = 0;
path.forEach( (p, i) => {
    if(i != 0){
        if(p[0] - path[i - 1][0] == 0 || p[1] - path[i - 1][1] == 0){
            pathLength += Math.abs(p[0] + p[1] - path[i - 1][0] - path[i - 1][1]);
        } else {
            pathLength += 78.53982;
        }
    }
});

function curCoords(percentage) {
}

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

function fillCircle(x, y, s, c){
    mapCtx.beginPath();
    mapCtx.lineWidth = 0.1;
    mapCtx.arc(x, y, s, 0, 2*Math.PI);
    mapCtx.fillStyle = c;
    mapCtx.fill();
    mapCtx.stroke();
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

    // travel path
    mapCtx.strokeStyle = "#dc322f";
    mapCtx.lineWidth = 8;
    mapCtx.lineCap = "square";
    mapCtx.beginPath();
    mapCtx.moveTo(550, 525);
    mapCtx.lineTo(550, 575);
    mapCtx.stroke();

    mapCtx.beginPath();
    mapCtx.moveTo(...path[0]);
    path.forEach( (p, i) => {
        if(i != 0){
            if(p[0] - path[i - 1][0] == 0 || p[1] - path[i - 1][1] == 0){
                mapCtx.lineTo(...p);
            } else {
                let xx = (p[0] % 100 == 0 ? p[0] : path[i - 1][0]);
                let yy = (p[1] % 100 == 0 ? p[1] : path[i - 1][1]);
                let dx = p[0] - path[i - 1][0];
                let dy = p[1] - path[i - 1][1];
                let ax = p[0] - xx, ay = p[1] - yy;
                let bx = path[i - 1][0] - xx, by = path[i - 1][1] - yy;
                let cp = ax * by - bx * ay;
                if(cp < 0){
                    if(dx < 0 && dy > 0){
                        mapCtx.arc(xx, yy, 50, 0, Math.PI / 2);
                    } else if(dx < 0 && dy < 0) {
                        mapCtx.arc(xx, yy, 50, Math.PI / 2, Math.PI);
                    } else if(dx > 0 && dy < 0) {
                        mapCtx.arc(xx, yy, 50, Math.PI, 3 * Math.PI / 2);
                    } else if(dx > 0 && dy > 0) {
                        mapCtx.arc(xx, yy, 50, 3 * Math.PI / 2, 0);
                    }
                } else {
                    if(dx < 0 && dy > 0){
                        mapCtx.arc(xx, yy, 50, 3 * Math.PI / 2, Math.PI, true);
                    } else if(dx < 0 && dy < 0) {
                        mapCtx.arc(xx, yy, 50, 0, 3 * Math.PI / 2, true);
                    } else if(dx > 0 && dy < 0) {
                        mapCtx.arc(xx, yy, 50, Math.PI / 2, 0, true);
                    } else if(dx > 0 && dy > 0) {
                        mapCtx.arc(xx, yy, 50, Math.PI, Math.PI / 2, true);
                    }
                }
            }
        }
    });
    mapCtx.stroke();

    // target
    fillCircle(350, 350, 28, "#268bd2");
    fillCircle(350, 350, 21, "#002b36");
    fillCircle(350, 350, 14, "#dc322f");
    fillCircle(350, 350, 7, "#002b36");

    // path tracer
    fillCircle(550, 550, 6, "#859900");

    // robot path
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
