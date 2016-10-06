    commands: /* empty */
            | commands command
            ;

    command:
            heat_switch
            |
            target_set
            ;

    heat_switch:
            TOKHEAT STATE
            {
                    printf("\tHeat turned on or off\n");
            }
            ;

    target_set:
            TOKTARGET TOKTEMPERATURE NUMBER
            {
                    printf("\tTemperature set\n");
            }
            ;
