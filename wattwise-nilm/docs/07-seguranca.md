# 07 — Segurança ⚠️

Este projeto conecta‑se à **rede elétrica (127/220/230 V)**. Erros aqui causam choque,
incêndio ou morte. Leia tudo antes de energizar.

## Regras não‑negociáveis

1. **Medição de corrente é sempre por indução (CT clamp).** O sensor abraça o fio; não
   há contato elétrico com o condutor de rede. Nunca meça corrente cortando o circuito
   com este projeto.
2. **Medição de tensão é isolada** (ZMPT101B / transformador). Nunca ligue divisor
   resistivo direto da rede no ADC.
3. **Fonte AC‑DC isolada e certificada** (HLK‑5M05). Não improvise fonte não isolada.
4. **Barreira de isolação na PCB** ≥ 6,4 mm HV↔LV, com slots. Ver `04-pcb.md`.
5. **Proteção de entrada**: fusível + MOV (varistor) + NTC de inrush.
6. **Instalação no quadro** deve ser feita por eletricista habilitado, com o circuito
   desenergizado. O secundário de um CT **nunca** pode ficar em aberto com corrente no
   primário (sobretensão perigosa) — CTs do tipo SCT‑013 já têm burden/zener interno,
   mas mantenha sempre conectado ao AFE.
7. **Gabinete**: caixa fechada, isolante ou aterrada, com os bornes de rede protegidos.
   Nada de placa exposta perto da rede.

## Bring‑up seguro (ordem de energização)

1. Monte e teste **todo o lado de baixa tensão alimentado por USB‑C**, sem rede.
2. Valide MCU, I²C (RTC), SD, MQTT, ADC lendo sinais de teste (gerador de função).
3. Só então energize o lado de rede, **com transformador isolador de bancada** e
   corrente limitada, medindo isolação (megôhmetro) antes.
4. Verifique que o lado LV continua isolado (>10 MΩ) da rede após montagem.

## Conformidade (se for além do protótipo)

Um produto real precisaria de EMC/EMI (CISPR), segurança elétrica (IEC 62368/61010) e,
no Brasil, considerações de INMETRO/ANEEL para medição. Para uso pessoal/educacional,
mantenha o rigor de isolação acima.
