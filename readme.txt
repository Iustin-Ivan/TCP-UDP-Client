IVAN IUSTIN-GABRIEL 323CB

In primul rand nu inteleg diferentele dintre rularea manuala si cea pe checker. Testele de la final cum 
ar fi data_sf data_no_sf si alea pica dar cand rulez manual in terminal
./server port
./subscriber id ipserver portserver
python3 udp_client.py --source-port 1234 --input_file three_topics_payloads.json 
--mode random --delay 2000 127.0.0.1 8080
in terminalul cu subscriber dau subscribe topic_a 1
Se afiseaza ca e subscribed si apar acolo mesajele de la udp de unde e abonat 
dupa dau exit in terminalul cu subscriber si se inchide si astept o vreme sa se stranga mesaje
de la topicul unde e abonat si cand ma conectez iar cu acelasi id imi apar toate mesajele care
fusesera trimise de UDP intre timp. De aceea nu inteleg de ce nu trec testele de la final.
La tema asta cred ca ar merge un tutorial de debug ca sa pot intelege ce fac eu gresit de nu vrea 
la final sa imi dea checkerul passed macar sa inteleg ce fac gresit.
RESULTS
-------
compile...........................passed
server_start......................passed
c1_start..........................passed
data_unsubscribed.................passed
c1_subscribe_all..................passed
data_subscribed...................passed
c1_stop...........................passed
c1_restart........................passed
data_no_clients...................passed
same_id...........................passed
c2_start..........................passed
c2_subscribe......................passed
c2_subscribe_sf...................passed
data_no_sf........................failed
data_sf...........................failed
c2_stop...........................passed
data_no_sf_2......................failed
data_sf_2.........................failed
c2_restart_sf.....................passed
quick_flow..................not executed
server_stop.......................passed
la quick_flow am comentat eu linia din checker pentru ca statea prea mult pe testele alea doar 
ca sa dea timeout asa ca i-am dat skip dar in mod normal dadea failed si asa am eu pe local


Ca implementare server.cpp are initializare cu socket bind listen basic stuff. Am gandit sa fac
variabile globale pt ce folosesc pe tot parcursul programului si am nevoie de acces pe oriunde
si stochez in diferite structuri tot ce am nevoie packet respecta structura din pdf doar ca
i-am dat la payload mai mult pentru ca trimit tot mesajul de afisat in subscriber direct acolo
si am mai bagat 100B de siguranta ca sa nu am probleme cu bufferul.
Am facut o functie de parsare a mesajelor de la UDP si una de parsare a mesajelor de la TCP
Structura de client contine socketul clientului, starea de online sau offline si un vector de
topics la care e abonat care contine si el la randul lui un vector de mesaje de la topicurile
la care e abonat si parcurge tot ce are nevoie ca sa trimita ce trebuie unde trebuie
