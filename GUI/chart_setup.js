const C_RED = '#dc322f';
const C_ORANGE = '#b58900';
const C_YELLOW = '#859900';
const C_GREEN = '#268bd2';
const C_BLUE = '#6c71c4';
const C_PURPLE = '#d33682';
const COLORS = [C_RED, C_ORANGE, C_YELLOW, C_GREEN, C_BLUE, C_PURPLE];

let charts = [];
function buildChart(title, name, inputs){
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
                elements: {
                    point: {
                        radius: 0,
                    },
                },
                plugins: {
                    legend: {
                        labels: {
                            boxWidth: 30,
                        },
                    },
                    title: {
                        display: true,
                        text: title,
                        font: {
                            color: '#839496',
                        },
                        padding: 0,
                    },
                },
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
    return chart;
}
let enc  = buildChart('Left and Right Encoder Counts', 'encChart', ['mel', 'mer']);
let speL = buildChart('Left Target and Actual Speed', 'speChartL', ['mtl', 'msl']);
let speR = buildChart('Right Target and Actual Speed', 'speChartR', ['mtr', 'msr']);
let bav  = buildChart('Battery Voltage', 'bavChart', ['bav']);
let sta  = buildChart('Current State', 'staChart', ['sta']);
sta.config.options.scales.y.beginAtZero = true;
sta.config.options.scales.y.max = 18;
let pho  = buildChart('Photo Diodes', 'phoChart', ['pd0', 'pd1', 'pd2', 'pd3', 'pd4', 'pd5']);
let pdwa  = buildChart('Weight Average of Photo Diodes', 'pdwaChart', ['pdwa']);
let pda  = buildChart('Average of Photo Diodes', 'pdaChart', ['pda']);
