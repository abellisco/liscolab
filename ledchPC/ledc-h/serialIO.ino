
int ltrim (char * line ) {
  int idx0 = 0;
  while (( line[idx0] == ' ' ) && ( idx0 < LINE_BUF_SIZE )) {
    idx0++;
  }
  return idx0;
}


void $Serial ( void* parameter) {
  char logmsg[LINE_BUF_SIZE];
  String line_string;
  char key[] = ";";
  char * pch, * pch1;
  int n, x;

  for (;;) {
    if (Serial.available()) {
      line_string = Serial.readStringUntil( '\n' );
      if (line_string.length() < LINE_BUF_SIZE) {
        line_string.trim();
        line_string.toCharArray( vcharline, LINE_BUF_SIZE );
        pch = strpbrk ( vcharline, key );
        if (pch != NULL) {
          pch1 = strtok ( vcharline, key );
          while ( pch1 != NULL )
          {
            strcpy ( cline , pch1 );
            n = ltrim ( cline );
            x = 0;
            while (cline[n] != NULL ) {
              csend[x] = cline[n];
              x++;
              n++;
            }
            csend[x] = NULL; //'\0';
            xQueueSend(xQSerialIn, csend, NULL);
            Serial.println( csend );
            pch1 = strtok ( NULL, key );
          }
        }
        else {
          xQueueSend( xQSerialIn, vcharline, NULL );
          Serial.println( vcharline );
        }

      } else {
        Serial.println( "error: input line too long" );
      }
    } else {
      vTaskDelay (500 / portTICK_PERIOD_MS);
    }
  }
}

void prvOut() {

  Serial.println(cOutBuffer);
  memset ( cOutBuffer, NULL, configCOMMAND_INT_MAX_OUTPUT_SIZE);

}

void $Spool (void*) {
  char cxspool[500];
  for (;;) {
    if (xQueueReceive(xQSpooler, cxspool , portMAX_DELAY)) {
      Serial.println(cxspool);
    }
  }
}
