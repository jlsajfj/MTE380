from flask import Flask
from flask_socketio import SocketIO, emit
from celery import Celery

app = Flask(__name__)
app.config["SECRET_KEY"] = "there is no secret"
app.config.update(CELERY_BROKER_URL="pyamqp://", CELERY_RESULT_BACKEND="rpc://")
socketio = SocketIO(app, debug=True, cors_allowed_origins="*", async_mode="eventlet")
celery = Celery(
    app.name,
    broker=app.config["CELERY_BROKER_URL"],
    backend=app.config["CELERY_RESULT_BACKEND"],
)
celery.conf.update(app.config)


@celery.task
def update_data(tis, mel):
    socketio.emit("cpu", {"name": tis, "value": mel})
