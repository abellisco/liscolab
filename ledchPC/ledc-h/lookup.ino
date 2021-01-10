
BaseType_t exactfind(const table_str *table, const char * str2, esp_err_t *OpResult ) {

  bool endoftable = false, found = false;
  uint16_t n = 0;

  while ((!endoftable) || (!found) ) {
    if (strcmp (table[n].idxname, str2) == 0) {
      found = true;
    } else if (table[n].idxname == NULL) {
      endoftable = true;
    } else {
      n++;
    }
  }

  *OpResult = ((!endoftable) && (found)) ? ESP_OK : ESP_ERR_NOT_FOUND;
  return n;
}

/*---------------------------------------------------------------------------*/
uint8_t usParamlookup (char pccmd, const struct cmd_table_t *ptr, bool *lookupResult ) {
  *lookupResult = false;

  while (ptr->cmd_token != NULL) {
    if (ptr->cmd_token == pccmd) {
      *lookupResult = true;
      return ptr->ivalue;
    }
    else {
      ptr++;
    }
  };
  *lookupResult = false;
  return ptr->ivalue;
}// end lookup

/*----------------------------------------------------------------*/

uint16_t uvLookupTable (uint16_t ParamNbr, const char *pcCommand, const cmd_table_t *ParamList, esp_err_t *OpResult) {
  const char *cParam, *pcline;
  uint16_t IntResult;
  BaseType_t ParamLength;
  pcline = pcCommand;
  bool lookupResult;

  cParam = FreeRTOS_CLIGetParameter( pcCommand, (UBaseType_t) ParamNbr, &ParamLength );
  if (ParamLength > 1) {
    *OpResult = (esp_err_t)ESP_ERR_INVALID_SIZE;
/*    Serial.printf("error Parameter %d \r\n", ParamNbr);
        Serial.println(esp_err_to_name(*OpResult)); */
    return NULL;
  }
  else {
    char P = *cParam;
/*    Serial.printf("Param:%s ", cParam);   */
    IntResult = usParamlookup(P, ParamList, &lookupResult);
    *OpResult = (lookupResult) ? ESP_OK : ESP_ERR_NOT_FOUND;
    if (*OpResult != ESP_OK) {
      return NULL;
    } else {
      /*      Serial.printf("Param#%d %c -> %d ", ParamNbr, P, IntResult);*/
      return IntResult;
    }
  }
}
/*-----------------------------------------------------------*/
uint16_t uvParamStr2uInt (const char *pcCommandString, UBaseType_t IntParam) {
  BaseType_t pxParamStringLength ;
  const char *pcgralptr;
  char cgralbuffer[7];
  int16_t uvalue;

  pcgralptr = FreeRTOS_CLIGetParameter( pcCommandString, (UBaseType_t) IntParam, &pxParamStringLength );
  strncpy (cgralbuffer, pcgralptr, pxParamStringLength);
  cgralbuffer[pxParamStringLength] = NULL;
  uvalue = atoi(cgralbuffer);
  return uvalue;
}
/*-----------------------------------------------------------*/
uint16_t ivParamStr2signedInt (const char *pcCommandString, UBaseType_t signedIntParam) {
  BaseType_t pxParamStringLength ;
  const char *pcgralptr;
  char cgralbuffer[7];
  int16_t ivalue;

  pcgralptr = FreeRTOS_CLIGetParameter( pcCommandString, (UBaseType_t) signedIntParam, &pxParamStringLength );
  strncpy (cgralbuffer, pcgralptr, pxParamStringLength);
  cgralbuffer[pxParamStringLength] = NULL;
  ivalue = atoi(cgralbuffer);
  return ivalue;
}
/*-----------------------------------------------------------*/
uint32_t uvParamStr2u32Int (const char *pcCommandString, UBaseType_t IntParam) {
  BaseType_t pxParamStringLength ;
  const char *pcgralptr;
  char cgralbuffer[7];
  uint32_t u32value;

  pcgralptr = FreeRTOS_CLIGetParameter( pcCommandString, (UBaseType_t) IntParam, &pxParamStringLength );
  strncpy (cgralbuffer, pcgralptr, pxParamStringLength);
  cgralbuffer[pxParamStringLength] = NULL;
  u32value = atol(cgralbuffer);
  return u32value;
}

/*-----------------------------------------------------------*/
double uvParamStr2double (const char *pcCommandString, UBaseType_t IntParam) {
  BaseType_t pxParamStringLength ;
  const char *pcgralptr;
  char cgralbuffer[15];
  double dvalue;

  pcgralptr = FreeRTOS_CLIGetParameter( pcCommandString, (UBaseType_t) IntParam, &pxParamStringLength );
  strncpy (cgralbuffer, pcgralptr, pxParamStringLength);
  cgralbuffer[pxParamStringLength] = NULL;
  dvalue = strtod( cgralbuffer , NULL );
  return dvalue;
}
