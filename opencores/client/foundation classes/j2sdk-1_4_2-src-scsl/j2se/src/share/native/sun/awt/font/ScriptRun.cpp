/*
 * @(#)ScriptRun.cpp	1.1 01/03/23
 *
 * (C) Copyright IBM Corp. 2001 - All Rights Reserved
 *
 */

#include "LETypes.h"
#include "ScriptRun.h"

le_bool ScriptRun::sameScript(le_int32 scriptOne, le_int32 scriptTwo)
{
	return scriptOne <= 0 || scriptTwo <= 0 || scriptOne == scriptTwo;
}

le_int32 ScriptRun::getScriptCode(LEUnicode32 ch)
{
    le_int32 probe = scriptRecordsPower;
    le_int32 index = 0;

    if (ch >= scriptRecords[scriptRecordsExtra].startChar) {
        index = scriptRecordsExtra;
    }

    while (probe > (1 << 0)) {
        probe >>= 1;

        if (ch >= scriptRecords[index + probe].startChar) {
            index += probe;
        }
    }

	if (ch >= scriptRecords[index].startChar && ch <= scriptRecords[index].endChar) {
		return scriptRecords[index].scriptCode;
	}

    return -1;
}

le_bool ScriptRun::next()
{
    if (scriptEnd >= charLimit) {
        return false;
    }
    
    scriptCode = -1;

    for (scriptStart = scriptEnd; scriptEnd < charLimit; scriptEnd += 1) {
        LEUnicode16 high = charArray[scriptEnd];
		LEUnicode32 ch = high;

		if (scriptEnd < charLimit - 1 && high >= 0xD800 && high <= 0xDBFF)
		{
			LEUnicode16 low = charArray[scriptEnd + 1];

			if (low >= 0xDC00 && low <= 0xDFFF) {
				ch = (high - 0xD800) * 0x0400 + low - 0xDC00 + 0x10000;
				scriptEnd += 1;
			}
		}

        le_int32 sc = getScriptCode(ch);

        if (sameScript(scriptCode, sc)) {
            if (scriptCode <= 0 && sc > 0) {
                scriptCode = sc;
            }
        } else {
            break;
        }
    }

    return true;
}

const le_int16 *ScriptRun::getScriptRangeOffsets(le_int32 scriptCode)
{
	return scriptRangeOffsets[scriptCode];
}

const ScriptRecord *ScriptRun::getScriptRecord(le_int16 offset)
{
	return &scriptRecords[offset];
}

