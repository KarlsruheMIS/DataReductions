
// board
let board;
let cX, cY;
let ctx;

// Simulation
let X, Y;
let vX, vY;
let fX, fY;

let momentum = 0.9;
let eps = 0.0001;
let spring = 0.1;
let center_g = 0.01;

// Interactions
let zoom = 1.0;
let zc = 0.5;
let oX = 0.0, oY = 0.0;

let mS = -1, S;

window.onload = function () {
    board = document.getElementById("board");
    board.height = window.innerHeight - 50;
    board.width = window.innerWidth - 50;
    cX = board.width / 2, cY = board.height / 2;
    ctx = board.getContext("2d");

    load_graph();
    draw_graph();
    setInterval(update, 32);
}

function load_graph() {

    X = new Array(n), Y = new Array(n);
    vX = new Array(n), vY = new Array(n);
    fX = new Array(n), fY = new Array(n);
    S = new Array(n);

    for (let i = 0; i < n; i++) {
        X[i] = Math.random() * 100, Y[i] = Math.random() * 100;

        vX[i] = 0, vY[i] = 0;
        fX[i] = 0, fY[i] = 0;
        S[i] = 0;
    }
}

function update() {
    for (let i = 0; i < n; i++) {
        fX[i] = 0;
        fY[i] = 0;
    }

    for (let u = 0; u < n; u++) {

        fX[u] += -X[u] * center_g;
        fY[u] += -Y[u] * center_g;

        for (let v = u + 1; v < n; v++) {
            let dist = Math.sqrt((X[u] - X[v]) ** 2 + (Y[u] - Y[v]) ** 2) * 0.01;

            fX[u] += (X[u] - X[v]) / (dist ** 2 + eps);
            fY[u] += (Y[u] - Y[v]) / (dist ** 2 + eps);

            fX[v] += (X[v] - X[u]) / (dist ** 2 + eps);
            fY[v] += (Y[v] - Y[u]) / (dist ** 2 + eps);
        }

        for (let i = V[u]; i < V[u + 1]; i++) {
            let v = E[i];
            let dist = Math.sqrt((X[u] - X[v]) ** 2 + (Y[u] - Y[v]) ** 2);

            fX[u] += (X[v] - X[u]) * spring;
            fY[u] += (Y[v] - Y[u]) * spring;
        }
    }

    for (let u = 0; u < n; u++) {
        if (mS == u) continue;

        vX[u] = vX[u] * momentum + fX[u] * (1 - momentum);
        vY[u] = vY[u] * momentum + fY[u] * (1 - momentum);

        X[u] += vX[u];
        Y[u] += vY[u];
    }

    draw_graph();
}

function draw_graph() {
    ctx.clearRect(0, 0, board.width, board.height);

    ctx.fillStyle = "black";
    ctx.strokeStyle = "black";
    ctx.lineWidth = 2;

    for (let u = 0; u < n; u++) {
        let x = cX + (X[u] + oX) * zoom * zc, y = cY + (Y[u] + oY) * zoom * zc;

        ctx.beginPath();
        ctx.arc(x, y, 10, 0, 2 * Math.PI);
        ctx.fill();
        ctx.stroke();

        for (let i = V[u]; i < V[u + 1]; i++) {
            let v = E[i];
            if (u < v)
                continue;

            let vx = cX + (X[v] + oX) * zoom * zc, vy = cY + (Y[v] + oY) * zoom * zc;

            ctx.beginPath();
            ctx.moveTo(0, 0);
            ctx.moveTo(x, y);
            ctx.lineTo(vx, vy);
            ctx.stroke();
        }
    }

    ctx.font = "25px Arial";
    ctx.fillStyle = "white";

    for (let u = 0; u < n; u++) {
        let x = cX + (X[u] + oX) * zoom * zc, y = cY + (Y[u] + oY) * zoom * zc;
        ctx.fillText(u.toString() + "(" + W[u].toString() + ")", x, y);
    }
}

let md = 0;

addEventListener("mousemove", (event) => {
    if (md == 1 && mS < 0) {
        oX += event.movementX * 1.0 / (zoom * zc);
        oY += event.movementY * 1.0 / (zoom * zc);
    } else if (md && mS >= 0) {
        X[mS] += event.movementX * 1.0 / (zoom * zc);
        Y[mS] += event.movementY * 1.0 / (zoom * zc);
    }
});

addEventListener("wheel", (event) => {
    if (!md) {
        zoom -= event.deltaY * 0.0005;
        if (zoom < eps) zoom = eps;
    }
});


addEventListener("mousedown", (event) => {

    md = 1, mS = -1;

    let rect = board.getBoundingClientRect();
    let mx = event.clientX - rect.left;
    let my = event.clientY - rect.top;

    for (let u = 0; u < n; u++) {
        let x = cX + (X[u] + oX) * zoom * zc, y = cY + (Y[u] + oY) * zoom * zc;
        let dist = Math.sqrt((x - mx) ** 2 + (y - my) ** 2);
        if (dist < 10) {
            mS = u;
            S[u] = 1;
            break;
        }
    }
});

addEventListener("mouseup", (event) => {
    md = 0;
    if (mS >= 0) {
        S[mS] = 0;
        mS = -1;
    }
});