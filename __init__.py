from flask import Flask, render_template, request, json
import os

app = Flask(__name__)

@app.route("/")
def index():
    return render_template('index.html')

@app.route('/criar')
def criar():
    try:
        nome = request.args.get("nome")
        memoria = request.args.get("memoria")
        cpus = request.args.get("quantidade_cpus")
        print ("Nome: " + nome, "Memoria: " + memoria, "CPUs: "+ cpus)
        os.chdir("/home/lucas/flask_vm/")
        os.system("bash ./create_vm.sh " + nome + " " + cpus + " " + memoria)
        return render_template('criado.html')
        
    except Exception as e:
        print("Erro CRIAR")
        return json.dumps({'error':str(e)})

if __name__ == "__main__":
    app.run(debug=True)
