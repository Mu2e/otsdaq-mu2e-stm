from flask import Flask, render_template
import psycopg2
import argparse,glob,os,sys,subprocess,datetime,time,requests,pytz
import matplotlib.dates as mdate
from collections import defaultdict
import numpy as np
from dateutil import parser
import os

xparser = argparse.ArgumentParser(description='Parameters to g-2 efficiency/status plots')
xparser.add_argument('--db', type=str, required=True, dest='db', choices=['localhost','g2db-priv'], help='database connection')
args = xparser.parse_args()
db = args.db
app = Flask(__name__)


@app.route("/simple_plot")
def simple_plot():
    text = "<h2> Hello world </h2>"
    return render_template('displayMain.html', user_text=text)

@app.route("/")
def home():
    return render_template('displayMain.html', user_text="<p> <br><br><br>Use the menu on the left to display plots </p>")

IMG_DIR = os.getenv('STMDAQ_ROOT')+"dqm/Flask/static"

if __name__ == "__main__":
    app.config.update(
        DEBUG=True)
    app.run('0.0.0.0',port=5002)
