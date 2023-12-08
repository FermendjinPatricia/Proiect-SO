#!/bin/bash

if [ $# -ne 1 ]
then echo "Numar invalid de argumente."
exit 1
fi

contor=0

while read linie
do
    if echo $linie | grep -q "^[A-Z][A-Za-z0-9, .?!]*[.?!]$"; then # cerinta de propozitie corecta
        if echo $linie | grep -q "$1"; then                        # sa contina caracterul dat ca parametru 
            if echo $linie | grep -v -q ",si"; then                # sa nu contina virgula inainte de si 
                contor=$(($contor + 1))
            fi
        fi
    fi
done


echo $contor