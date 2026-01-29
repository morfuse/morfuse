#pragma once

#include "Asset.h"
#include "Vector.h"
#include "Container/Container.h"
#include "Common/str.h"

#define TOKENCOMMENT        (';')
#define TOKENCOMMENT2        ('#')
#define TOKENEOL            ('\n')
//#define TOKENNULL            ('\0')
#define TOKENSPACE            (' ')
#define TOKENSPECIAL        ('$')

#define    MAXTOKEN    512

namespace mfuse
{
    class Script : public Asset
    {
    public:
        typedef struct
        {
            bool tokenready;
            size_t offset;
            int32_t line;
            str token;
        } scriptmarker_t;


        struct macro
        {
            str macroName;
            str macroText;
        };
    protected:
        bool        tokenready;

        str filename;
        const char    *script_p;
        const char    *end_p;
        Container<macro *> macrolist;

        int32_t line;
        str token;

        bool        releaseBuffer;

        bool        AtComment(void);
        void            CheckOverflow(void);

    public:
        const char    *buffer;
        uint64_t    length;

        mfuse_EXPORTS ~Script();
        mfuse_EXPORTS Script(const char* filename);
        mfuse_EXPORTS Script();

        mfuse_EXPORTS void Load() override;
        mfuse_EXPORTS void Close(void);
        mfuse_EXPORTS const char* Filename(void);
        mfuse_EXPORTS int GetLineNumber(void);
        mfuse_EXPORTS void Reset(void);
        mfuse_EXPORTS void MarkPosition(scriptmarker_t* mark);
        mfuse_EXPORTS void RestorePosition(const scriptmarker_t* mark);
        mfuse_EXPORTS bool SkipToEOL(void);
        mfuse_EXPORTS void SkipWhiteSpace(bool crossline);
        mfuse_EXPORTS void SkipNonToken(bool crossline);
        mfuse_EXPORTS bool TokenAvailable(bool crossline);
        mfuse_EXPORTS bool CommentAvailable(bool crossline);
        mfuse_EXPORTS void UnGetToken(void);
        mfuse_EXPORTS bool AtString(bool crossline);
        mfuse_EXPORTS bool AtOpenParen(bool crossline);
        mfuse_EXPORTS bool AtCloseParen(bool crossline);
        mfuse_EXPORTS bool AtComma(bool crossline);
        mfuse_EXPORTS bool AtDot(bool crossline);
        mfuse_EXPORTS bool AtAssignment(bool crossline);
        mfuse_EXPORTS const char* GetToken(bool crossline);
        mfuse_EXPORTS const char* GetLine(bool crossline);
        mfuse_EXPORTS const char* GetRaw(void);
        mfuse_EXPORTS const char* GetString(bool crossline, bool allowMultiLines = false);
        mfuse_EXPORTS bool GetSpecific(const char* string);
        mfuse_EXPORTS bool GetBoolean(bool crossline);
        mfuse_EXPORTS int GetInteger(bool crossline);
        mfuse_EXPORTS double GetDouble(bool crossline);
        mfuse_EXPORTS float GetFloat(bool crossline);
        mfuse_EXPORTS Vector GetVector(bool crossline);
        mfuse_EXPORTS int LinesInFile(void);
        mfuse_EXPORTS void Parse(const char* data, uintmax_t length, const char* name = "");
        mfuse_EXPORTS void LoadFile(const char* name);
        mfuse_EXPORTS void LoadFile(const char* name, int length, const char* buf);
        mfuse_EXPORTS const char* Token(void);
        mfuse_EXPORTS void AddMacroDefinition(bool crossline);
        mfuse_EXPORTS const char* GetMacroString(const char* theMacroName);
        mfuse_EXPORTS char* EvaluateMacroString(const char* theMacroString);
        mfuse_EXPORTS double EvaluateMacroMath(double value, double newval, char oper);
        mfuse_EXPORTS const char* GetExprToken(const char* ptr, char* token);
        mfuse_EXPORTS const char* GrabNextToken(bool crossline);
        mfuse_EXPORTS bool isMacro(void);

        mfuse_EXPORTS bool EndOfFile();
        mfuse_EXPORTS bool isValid(void);

        mfuse_EXPORTS Container<macro *> *GetMacroList() { return &macrolist; }
        mfuse_EXPORTS void AddMacro(const char *name, const char *value);
    };
};
