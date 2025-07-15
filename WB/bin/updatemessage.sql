.open idb+tcp://127.0.0.1/butler.db
UPDATE Setting SET infomessage = 'Bitte warten, das System bekommmt ein Update. NICHT ABSCHALTEN!';
commit;
.close
