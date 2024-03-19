from celery import Celery
import struct

app = Celery('writer', backend='rpc://', broker='pyamqp://')

@app.task
def ack(data: None):
    pass

@app.task
def nack(data: None):
    pass

@app.task
def stream(data_stream: bytes):
    data = dict(
        zip(
            [
                "msl",
                "msr",
                "mtl",
                "mtr",
                "mel",
                "mer",
                "sta",
                "pd0",
                "pd1",
                "pd2",
                "pd3",
                "pd4",
                "pd5",
                "bav",
                "mag",
                "tis",
            ],
            struct.unpack("<2b2b2iB6BBBI", data_stream),
        )
    )
    print(data)

@app.task
def config(data: dict[str,any]):
    print(config)
