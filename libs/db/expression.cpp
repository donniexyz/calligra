/* This file is part of the KDE project
   Copyright (C) 2003-2015 Jarosław Staniek <staniek@kde.org>

   Based on nexp.cpp : Parser module of Python-like language
   (C) 2001 Jarosław Staniek, MIMUW (www.mimuw.edu.pl)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "expression.h"
#include "utils.h"
#include "parser/sqlparser.h"
#include "parser/parser_p.h"

#include <ctype.h>

#include <kdebug.h>
#include <klocale.h>

CALLIGRADB_EXPORT QString KexiDB::exprClassName(int c)
{
    if (c == KexiDBExpr_Unary)
        return "Unary";
    else if (c == KexiDBExpr_Arithm)
        return "Arithm";
    else if (c == KexiDBExpr_Logical)
        return "Logical";
    else if (c == KexiDBExpr_Relational)
        return "Relational";
    else if (c == KexiDBExpr_SpecialBinary)
        return "SpecialBinary";
    else if (c == KexiDBExpr_Const)
        return "Const";
    else if (c == KexiDBExpr_Variable)
        return "Variable";
    else if (c == KexiDBExpr_Function)
        return "Function";
    else if (c == KexiDBExpr_Aggregation)
        return "Aggregation";
    else if (c == KexiDBExpr_TableList)
        return "TableList";
    else if (c == KexiDBExpr_ArgumentList)
        return "ArgumentList";
    else if (c == KexiDBExpr_QueryParameter)
        return "QueryParameter";

    return "Unknown";
}

using namespace KexiDB;

//=========================================

BaseExpr::BaseExpr(int token)
        : m_cl(KexiDBExpr_Unknown)
        , m_par(0)
        , m_token(token)
{
}

BaseExpr::~BaseExpr()
{
}

Field::Type BaseExpr::type()
{
    return Field::InvalidType;
}

bool BaseExpr::isTextType()
{
    return Field::isTextType(type());
}

bool BaseExpr::isIntegerType()
{
    return Field::isIntegerType(type());
}

bool BaseExpr::isNumericType()
{
    return Field::isNumericType(type());
}

bool BaseExpr::isFPNumericType()
{
    return Field::isFPNumericType(type());
}

bool BaseExpr::isDateTimeType()
{
    return Field::isDateTimeType(type());
}

QString BaseExpr::debugString()
{
    return QString("BaseExpr(%1,type=%1)").arg(m_token).arg(Driver::defaultSQLTypeName(type()));
}

bool BaseExpr::validate(ParseInfo& /*parseInfo*/)
{
    return true;
}

QString BaseExpr::tokenToDebugString(int token)
{
    if (token < 254) {
        if (isprint(token))
            return QString(QChar(uchar(token)));
        else
            return QString::number(token);
    }
    return QLatin1String(tokenName(token));
}

QString BaseExpr::tokenToString()
{
    if (m_token < 255 && isprint(m_token))
        return tokenToDebugString();
    return QString();
}

NArgExpr* BaseExpr::toNArg()
{
    return dynamic_cast<NArgExpr*>(this);
}
UnaryExpr* BaseExpr::toUnary()
{
    return dynamic_cast<UnaryExpr*>(this);
}
BinaryExpr* BaseExpr::toBinary()
{
    return dynamic_cast<BinaryExpr*>(this);
}
ConstExpr* BaseExpr::toConst()
{
    return dynamic_cast<ConstExpr*>(this);
}
VariableExpr* BaseExpr::toVariable()
{
    return dynamic_cast<VariableExpr*>(this);
}
FunctionExpr* BaseExpr::toFunction()
{
    return dynamic_cast<FunctionExpr*>(this);
}
QueryParameterExpr* BaseExpr::toQueryParameter()
{
    return dynamic_cast<QueryParameterExpr*>(this);
}

//=========================================

NArgExpr::NArgExpr(int aClass, int token)
        : BaseExpr(token)
{
    m_cl = aClass;
}

NArgExpr::NArgExpr(const NArgExpr& expr)
        : BaseExpr(expr)
{
    foreach(BaseExpr* e, expr.list) {
        add(e->copy());
    }
}

NArgExpr::~NArgExpr()
{
    qDeleteAll(list);
}

NArgExpr* NArgExpr::copy() const
{
    return new NArgExpr(*this);
}

Field::Type NArgExpr::type()
{
    switch (m_token) {
    case KEXIDB_TOKEN_BETWEEN_AND:
    case KEXIDB_TOKEN_NOT_BETWEEN_AND:
        foreach (BaseExpr* e, list) {
            Field::Type type = e->type();
            if (type == Field::InvalidType || type == Field::Null) {
                return type;
            }
        }

        return Field::Boolean;
    default:;
    }

    return BaseExpr::type();
}

bool NArgExpr::containsInvalidArgument()
{
    foreach (BaseExpr* e, list) {
        Field::Type type = e->type();
        if (type == Field::InvalidType) {
            return true;
        }
    }
    return false;
}

bool NArgExpr::containsNullArgument()
{
    foreach (BaseExpr* e, list) {
        Field::Type type = e->type();
        if (type == Field::Null) {
            return true;
        }
    }
    return false;
}

QString NArgExpr::debugString()
{
    QString s = QString("NArgExpr(")
                + tokenToString() + ", " + "class=" + exprClassName(m_cl);
    foreach(BaseExpr *expr, list) {
        s += ", " +
             expr->debugString();
    }
    s += QString::fromLatin1(",type=%1)").arg(Driver::defaultSQLTypeName(type()));
    return s;
}

QString NArgExpr::toString(QuerySchemaParameterValueListIterator* params)
{
    if (BaseExpr::token() == KEXIDB_TOKEN_BETWEEN_AND && list.count() == 3) {
        return list[0]->toString() + " BETWEEN " + list[1]->toString() + " AND " + list[2]->toString();
    }
    if (BaseExpr::token() == KEXIDB_TOKEN_NOT_BETWEEN_AND && list.count() == 3) {
        return list[0]->toString() + " NOT BETWEEN " + list[1]->toString() + " AND " + list[2]->toString();
    }

    QString s;
    s.reserve(256);
    foreach(BaseExpr* e, list) {
        if (!s.isEmpty())
            s += ", ";
        s += e->toString(params);
    }
    return s;
}

void NArgExpr::getQueryParameters(QuerySchemaParameterList& params)
{
    foreach(BaseExpr *e, list) {
        e->getQueryParameters(params);
    }
}

BaseExpr* NArgExpr::arg(int nr)
{
    return list.at(nr);
}

void NArgExpr::add(BaseExpr *expr)
{
    list.append(expr);
    expr->setParent(this);
}

void NArgExpr::prepend(BaseExpr *expr)
{
    list.prepend(expr);
    expr->setParent(this);
}

int NArgExpr::args()
{
    return list.count();
}

bool NArgExpr::validate(ParseInfo& parseInfo)
{
    if (!BaseExpr::validate(parseInfo))
        return false;

    foreach(BaseExpr *e, list) {
        if (!e->validate(parseInfo))
            return false;
    }

    switch (m_token) {
    case KEXIDB_TOKEN_BETWEEN_AND:
    case KEXIDB_TOKEN_NOT_BETWEEN_AND: {
        if (list.count() != 3) {
            parseInfo.errMsg = i18n("Incorrect number of arguments");
            parseInfo.errDescr = i18nc("@info BETWEEN..AND error", "%1 operator requires exactly three arguments.", "BETWEEN...AND");
            return false;
        }

        if (!(!Field::isNumericType(list[0]->type()) || !Field::isNumericType(list[1]->type()) || !Field::isNumericType(list[2]->type()))) {
            return true;
        } else if (!(!Field::isTextType(list[0]->type()) || !Field::isTextType(list[1]->type()) || !Field::isTextType(list[2]->type()))) {
            return true;
        }

        if ((list[0]->type() == list[1]->type() && list[1]->type() == list[2]->type())) {
            return true;
        }

        parseInfo.errMsg = i18n("Incompatible types of arguments");
        parseInfo.errDescr = i18nc("@info BETWEEN..AND type error", "%1 operator requires compatible types of arguments.", "BETWEEN..AND");

        return false;
    }
    default:;
    }

    return true;
}

QString NArgExpr::tokenToString()
{
    switch (m_token) {
    case KEXIDB_TOKEN_BETWEEN_AND: return "BETWEEN_AND";
    case KEXIDB_TOKEN_NOT_BETWEEN_AND: return "NOT_BETWEEN_AND";
    default: {
        const QString s = BaseExpr::tokenToString();
        if (!s.isEmpty()) {
            return QString("'%1'").arg(s);
        }
    }
    }
    return QString("{INVALID_N_ARG_OPERATOR#%1}").arg(m_token);
}

//=========================================
UnaryExpr::UnaryExpr(int token, BaseExpr *arg)
        : BaseExpr(token)
        , m_arg(arg)
{
    m_cl = KexiDBExpr_Unary;
    if (m_arg)
        m_arg->setParent(this);
}

UnaryExpr::UnaryExpr(const UnaryExpr& expr)
        : BaseExpr(expr)
        , m_arg(expr.m_arg ? expr.m_arg->copy() : 0)
{
    if (m_arg)
        m_arg->setParent(this);
}

UnaryExpr::~UnaryExpr()
{
    delete m_arg;
}

UnaryExpr* UnaryExpr::copy() const
{
    return new UnaryExpr(*this);
}

QString UnaryExpr::debugString()
{
    return "UnaryExpr('"
           + tokenToDebugString() + "', "
           + (m_arg ? m_arg->debugString() : QString("<NONE>"))
           + QString(",type=%1)").arg(Driver::defaultSQLTypeName(type()));
}

QString UnaryExpr::toString(QuerySchemaParameterValueListIterator* params)
{
    if (m_token == '(') //parentheses (special case)
        return '(' + (m_arg ? m_arg->toString(params) : "<NULL>") + ')';
    if (m_token < 255 && isprint(m_token))
        return tokenToDebugString() + (m_arg ? m_arg->toString(params) : "<NULL>");
    if (m_token == NOT)
        return "NOT " + (m_arg ? m_arg->toString(params) : "<NULL>");
    if (m_token == SQL_IS_NULL)
        return (m_arg ? m_arg->toString(params) : "<NULL>") + " IS NULL";
    if (m_token == SQL_IS_NOT_NULL)
        return (m_arg ? m_arg->toString(params) : "<NULL>") + " IS NOT NULL";
    return QString("{INVALID_OPERATOR#%1} ").arg(m_token) + (m_arg ? m_arg->toString(params) : "<NULL>");
}

void UnaryExpr::getQueryParameters(QuerySchemaParameterList& params)
{
    if (m_arg)
        m_arg->getQueryParameters(params);
}

Field::Type UnaryExpr::type()
{
    //NULL IS NOT NULL : BOOLEAN
    //NULL IS NULL : BOOLEAN
    switch (m_token) {
    case SQL_IS_NULL:
    case SQL_IS_NOT_NULL:
        return Field::Boolean;
    }
    const Field::Type t = m_arg->type();
    if (t == Field::Null)
        return Field::Null;
    if (m_token == NOT)
        return Field::Boolean;

    return t;
}

bool UnaryExpr::validate(ParseInfo& parseInfo)
{
    if (!BaseExpr::validate(parseInfo))
        return false;

    if (!m_arg->validate(parseInfo))
        return false;

//! @todo compare types... e.g. NOT applied to Text makes no sense...

    // update type
    if (m_arg->toQueryParameter()) {
        m_arg->toQueryParameter()->setType(type());
    }

    return true;
#if 0
    BaseExpr *n = l.at(0);

    n->check();
    /*typ wyniku:
        const bool dla "NOT <bool>" (negacja)
        int dla "# <str>" (dlugosc stringu)
        int dla "+/- <int>"
        */
    if (is(NOT) && n->nodeTypeIs(TYP_BOOL)) {
        node_type = new NConstType(TYP_BOOL);
    } else if (is('#') && n->nodeTypeIs(TYP_STR)) {
        node_type = new NConstType(TYP_INT);
    } else if ((is('+') || is('-')) && n->nodeTypeIs(TYP_INT)) {
        node_type = new NConstType(TYP_INT);
    } else {
        ERR("Niepoprawny argument typu '%s' dla operatora '%s'",
            n->nodeTypeName(), is(NOT) ? QString("not") : QChar(typ()));
    }
#endif
}

//=========================================
BinaryExpr::BinaryExpr(int aClass, BaseExpr *left_expr, int token, BaseExpr *right_expr)
        : BaseExpr(token)
        , m_larg(left_expr)
        , m_rarg(right_expr)
{
    m_cl = aClass;
    if (m_larg)
        m_larg->setParent(this);
    if (m_rarg)
        m_rarg->setParent(this);
}

BinaryExpr::BinaryExpr(const BinaryExpr& expr)
        : BaseExpr(expr)
        , m_larg(expr.m_larg ? expr.m_larg->copy() : 0)
        , m_rarg(expr.m_rarg ? expr.m_rarg->copy() : 0)
{
}

BinaryExpr::~BinaryExpr()
{
    delete m_larg;
    delete m_rarg;
}

BinaryExpr* BinaryExpr::copy() const
{
    return new BinaryExpr(*this);
}

bool BinaryExpr::validate(ParseInfo& parseInfo)
{
    if (!BaseExpr::validate(parseInfo))
        return false;

    if (!m_larg->validate(parseInfo))
        return false;
    if (!m_rarg->validate(parseInfo))
        return false;

//! @todo compare types..., BITWISE_SHIFT_RIGHT requires integers, etc...

    //update type for query parameters
    QueryParameterExpr * queryParameter = m_larg->toQueryParameter();
    if (queryParameter)
        queryParameter->setType(m_rarg->type());
    queryParameter = m_rarg->toQueryParameter();
    if (queryParameter)
        queryParameter->setType(m_larg->type());

    return true;
}

Field::Type BinaryExpr::type()
{
    const Field::Type lt = m_larg->type(), rt = m_rarg->type();
    if (lt == Field::InvalidType || rt == Field::InvalidType)
        return Field::InvalidType;
    if (lt == Field::Null || rt == Field::Null) {
        if (m_token != OR) //note that NULL OR something   != NULL
            return Field::Null;
    }

    switch (m_token) {
    case BITWISE_SHIFT_RIGHT:
    case BITWISE_SHIFT_LEFT:
    case CONCATENATION:
        return lt;
    }

    const bool ltInt = Field::isIntegerType(lt);
    const bool rtInt = Field::isIntegerType(rt);
    if (ltInt && rtInt)
        return KexiDB::maximumForIntegerTypes(lt, rt);

    if (Field::isFPNumericType(lt) && (rtInt || lt == rt))
        return lt;
    if (Field::isFPNumericType(rt) && (ltInt || lt == rt))
        return rt;

    return Field::Boolean;
}

QString BinaryExpr::debugString()
{
    return QLatin1String("BinaryExpr(")
           + "class=" + exprClassName(m_cl)
           + ',' + (m_larg ? m_larg->debugString() : QString::fromLatin1("<NONE>"))
           + ",'" + tokenToDebugString() + "',"
           + (m_rarg ? m_rarg->debugString() : QString::fromLatin1("<NONE>"))
           + QString::fromLatin1(",type=%1)").arg(Driver::defaultSQLTypeName(type()));
}

QString BinaryExpr::tokenToString()
{
    if (m_token < 255 && isprint(m_token))
        return tokenToDebugString();
    // other arithmetic operations: << >>
    switch (m_token) {
    case BITWISE_SHIFT_RIGHT: return ">>";
    case BITWISE_SHIFT_LEFT: return "<<";
        // other relational operations: <= >= <> (or !=) LIKE IN
    case NOT_EQUAL: return "<>";
    case NOT_EQUAL2: return "!=";
    case LESS_OR_EQUAL: return "<=";
    case GREATER_OR_EQUAL: return ">=";
    case LIKE: return "LIKE";
    case NOT_LIKE: return "NOT LIKE";
    case SQL_IN: return "IN";
        // other logical operations: OR (or ||) AND (or &&) XOR
    case SIMILAR_TO: return "SIMILAR TO";
    case NOT_SIMILAR_TO: return "NOT SIMILAR TO";
    case OR: return "OR";
    case AND: return "AND";
    case XOR: return "XOR";
        // other string operations: || (as CONCATENATION)
    case CONCATENATION: return "||";
        // SpecialBinary "pseudo operators":
        /* not handled here */
    default:;
    }
    return QString("{INVALID_BINARY_OPERATOR#%1} ").arg(m_token);
}

QString BinaryExpr::toString(QuerySchemaParameterValueListIterator* params)
{
#define INFIX(a) \
    (m_larg ? m_larg->toString(params) : "<NULL>") + ' ' + a + ' ' + (m_rarg ? m_rarg->toString(params) : "<NULL>")
    return INFIX(tokenToString());
}

void BinaryExpr::getQueryParameters(QuerySchemaParameterList& params)
{
    if (m_larg)
        m_larg->getQueryParameters(params);
    if (m_rarg)
        m_rarg->getQueryParameters(params);
}

//=========================================
ConstExpr::ConstExpr(int token, const QVariant& val)
        : BaseExpr(token)
        , value(val)
{
    m_cl = KexiDBExpr_Const;
}

ConstExpr::ConstExpr(const ConstExpr& expr)
        : BaseExpr(expr)
        , value(expr.value)
{
}

ConstExpr::~ConstExpr()
{
}

ConstExpr* ConstExpr::copy() const
{
    return new ConstExpr(*this);
}

Field::Type ConstExpr::type()
{
    if (m_token == SQL_NULL)
        return Field::Null;
    else if (m_token == INTEGER_CONST) {
//TODO ok?
//TODO: add sign info?
        if (value.type() == QVariant::Int || value.type() == QVariant::UInt) {
            qint64 v = value.toInt();
            if (v <= 0xff && v > -0x80)
                return Field::Byte;
            if (v <= 0xffff && v > -0x8000)
                return Field::ShortInteger;
            return Field::Integer;
        }
        return Field::BigInteger;
    } else if (m_token == CHARACTER_STRING_LITERAL) {
//TODO: Field::defaultTextLength() is hardcoded now!
        if (Field::defaultMaxLength() > 0
            && uint(value.toString().length()) > Field::defaultMaxLength())
        {
            return Field::LongText;
        }
        else
            return Field::Text;
    } else if (m_token == REAL_CONST)
        return Field::Double;
    else if (m_token == DATE_CONST)
        return Field::Date;
    else if (m_token == DATETIME_CONST)
        return Field::DateTime;
    else if (m_token == TIME_CONST)
        return Field::Time;

    return Field::InvalidType;
}

QString ConstExpr::debugString()
{
    return QString("ConstExpr('") + tokenToDebugString() + "'," + toString()
           + QString(",type=%1)").arg(Driver::defaultSQLTypeName(type()));
}

QString ConstExpr::toString(QuerySchemaParameterValueListIterator* params)
{
    Q_UNUSED(params);
    if (m_token == SQL_NULL)
        return "NULL";
    else if (m_token == CHARACTER_STRING_LITERAL)
//TODO: better escaping!
        return QLatin1Char('\'') + value.toString() + QLatin1Char('\'');
    else if (m_token == REAL_CONST)
        return QString::number(value.toPoint().x()) + QLatin1Char('.') + QString::number(value.toPoint().y());
    else if (m_token == DATE_CONST)
        return QLatin1Char('\'') + value.toDate().toString(Qt::ISODate) + QLatin1Char('\'');
    else if (m_token == DATETIME_CONST)
        return QLatin1Char('\'') + value.toDateTime().date().toString(Qt::ISODate)
               + QLatin1Char(' ') + value.toDateTime().time().toString(Qt::ISODate) + QLatin1Char('\'');
    else if (m_token == TIME_CONST)
        return QLatin1Char('\'') + value.toTime().toString(Qt::ISODate) + QLatin1Char('\'');

    return value.toString();
}

void ConstExpr::getQueryParameters(QuerySchemaParameterList& params)
{
    Q_UNUSED(params);
}

bool ConstExpr::validate(ParseInfo& parseInfo)
{
    if (!BaseExpr::validate(parseInfo))
        return false;

    return type() != Field::InvalidType;
}

//=========================================
QueryParameterExpr::QueryParameterExpr(const QString& message)
        : ConstExpr(QUERY_PARAMETER, message)
        , m_type(Field::Text)
{
    m_cl = KexiDBExpr_QueryParameter;
}

QueryParameterExpr::QueryParameterExpr(const QueryParameterExpr& expr)
        : ConstExpr(expr)
        , m_type(expr.m_type)
{
}

QueryParameterExpr::~QueryParameterExpr()
{
}

QueryParameterExpr* QueryParameterExpr::copy() const
{
    return new QueryParameterExpr(*this);
}

Field::Type QueryParameterExpr::type()
{
    return m_type;
}

void QueryParameterExpr::setType(Field::Type type)
{
    m_type = type;
}

QString QueryParameterExpr::debugString()
{
    return QString("QueryParameterExpr('") + QString::fromLatin1("[%2]").arg(value.toString())
           + QString("',type=%1)").arg(Driver::defaultSQLTypeName(type()));
}

QString QueryParameterExpr::toString(QuerySchemaParameterValueListIterator* params)
{
    return params ? params->getPreviousValueAsString(type()) : QString::fromLatin1("[%2]").arg(value.toString());
}

void QueryParameterExpr::getQueryParameters(QuerySchemaParameterList& params)
{
    QuerySchemaParameter param;
    param.message = value.toString();
    param.type = type();
    params.append(param);
}

bool QueryParameterExpr::validate(ParseInfo& parseInfo)
{
    Q_UNUSED(parseInfo);
    return type() != Field::InvalidType;
}

//=========================================
VariableExpr::VariableExpr(const QString& _name)
        : BaseExpr(0/*undefined*/)
        , name(_name)
        , field(0)
        , tablePositionForField(-1)
        , tableForQueryAsterisk(0)
{
    m_cl = KexiDBExpr_Variable;
}

VariableExpr::VariableExpr(const VariableExpr& expr)
        : BaseExpr(expr)
        , name(expr.name)
        , field(expr.field)
        , tablePositionForField(expr.tablePositionForField)
        , tableForQueryAsterisk(expr.tableForQueryAsterisk)
{
}

VariableExpr::~VariableExpr()
{
}

VariableExpr* VariableExpr::copy() const
{
    return new VariableExpr(*this);
}

QString VariableExpr::debugString()
{
    return QString("VariableExpr(") + name
           + QString(",type=%1)").arg(field ? Driver::defaultSQLTypeName(type()) : QString("FIELD NOT DEFINED YET"));
}

QString VariableExpr::toString(QuerySchemaParameterValueListIterator* params)
{
    Q_UNUSED(params);
    return name;
}

void VariableExpr::getQueryParameters(QuerySchemaParameterList& params)
{
    Q_UNUSED(params);
}

//! We're assuming it's called after VariableExpr::validate()
Field::Type VariableExpr::type()
{
    if (field)
        return field->type();

    //BTW, asterisks are not stored in VariableExpr outside of parser, so ok.
    return Field::InvalidType;
}

#define IMPL_ERROR(errmsg) parseInfo.errMsg = "Implementation error"; parseInfo.errDescr = errmsg

bool VariableExpr::validate(ParseInfo& parseInfo)
{
    if (!BaseExpr::validate(parseInfo))
        return false;
    field = 0;
    tablePositionForField = -1;
    tableForQueryAsterisk = 0;

    /* taken from parser's addColumn(): */
    KexiDBDbg << "checking variable name: " << name;
    QString tableName, fieldName;
    if (!KexiDB::splitToTableAndFieldParts(name, tableName, fieldName, KexiDB::SetFieldNameIfNoTableName)) {
        return false;
    }
//! @todo shall we also support db name?
    if (tableName.isEmpty()) {//fieldname only
        if (fieldName == "*") {
//   querySchema->addAsterisk( new QueryAsterisk(querySchema) );
            return true;
        }

        //find first table that has this field
        Field *firstField = 0;
        foreach(TableSchema *table, *parseInfo.querySchema->tables()) {
            Field *f = table->field(fieldName);
            if (f) {
                if (!firstField) {
                    firstField = f;
                } else if (f->table() != firstField->table()) {
                    //ambiguous field name
                    parseInfo.errMsg = i18n("Ambiguous field name");
                    parseInfo.errDescr = i18nc("@info",
                        "Both table <resource>%1</resource> and <resource>%2</resource> have "
                        "defined <resource>%3</resource> field. "
                        "Use <resource><placeholder>tableName</placeholder>.%4</resource> notation to specify table name.",
                        firstField->table()->name(), f->table()->name(),
                        fieldName, fieldName);
                    return false;
                }
            }
        }
        if (!firstField) {
            parseInfo.errMsg = i18n("Field not found");
            parseInfo.errDescr = i18n("Table containing \"%1\" field not found.", fieldName);
            return false;
        }
        //ok
        field = firstField; //store
//  querySchema->addField(firstField);
        return true;
    }

    //table.fieldname or tableAlias.fieldname
    TableSchema *ts = parseInfo.querySchema->table(tableName);
    int tablePosition = -1;
    if (ts) {//table.fieldname
        //check if "table" is covered by an alias
        const QList<int> tPositions = parseInfo.querySchema->tablePositions(tableName);
        QByteArray tableAlias;
        bool covered = true;
        foreach(int position, tPositions) {
            tableAlias = parseInfo.querySchema->tableAlias(position);
            if (tableAlias.isEmpty() || tableAlias.toLower() == tableName.toLatin1()) {
                covered = false; //uncovered
                break;
            }
            KexiDBDbg << " --" << "covered by " << tableAlias << " alias";
        }
        if (covered) {
            parseInfo.errMsg = i18n("Could not access the table directly using its name");
            parseInfo.errDescr = i18n("Table name <resource>%1</resource> is covered by aliases. "
                                      "Instead of <resource>%2</resource>, "
                                      "you can write <resource>%3</resource>.",
                                      tableName, tableName + "." + fieldName,
                                      tableAlias + "." + QString(fieldName));
            return false;
        }
        if (!tPositions.isEmpty()) {
            tablePosition = tPositions.first();
        }
    }
    else {//try to find tableAlias
        tablePosition = parseInfo.querySchema->tablePositionForAlias(tableName.toLatin1());
        if (tablePosition >= 0) {
            ts = parseInfo.querySchema->tables()->at(tablePosition);
            if (ts) {
//    KexiDBDbg << " --it's a tableAlias.name";
            }
        }
    }

    if (!ts) {
        parseInfo.errMsg = i18n("Table not found");
        parseInfo.errDescr = i18n("Unknown table \"%1\".", tableName);
        return false;
    }

    if (!parseInfo.repeatedTablesAndAliases.contains(tableName)) {  //for sanity
        IMPL_ERROR(tableName + "." + fieldName + ", !positionsList ");
        return false;
    }
    const QList<int> positionsList(parseInfo.repeatedTablesAndAliases.value(tableName));

    //it's a table.*
    if (fieldName == "*") {
        if (positionsList.count() > 1) {
            parseInfo.errMsg = i18n("Ambiguous \"%1.*\" expression", tableName);
            parseInfo.errDescr = i18n("More than one \"%1\" table or alias defined.", tableName);
            return false;
        }
        tableForQueryAsterisk = ts;
//   querySchema->addAsterisk( new QueryAsterisk(querySchema, ts) );
        return true;
    }

// KexiDBDbg << " --it's a table.name";
    Field *realField = ts->field(fieldName);
    if (!realField) {
        parseInfo.errMsg = i18n("Field not found");
        parseInfo.errDescr = i18n("Table \"%1\" has no \"%2\" field.", tableName, fieldName);
        return false;
    }

    // check if table or alias is used twice and both have the same column
    // (so the column is ambiguous)
    if (positionsList.count() > 1) {
        parseInfo.errMsg = i18n("Ambiguous \"%1.%2\" expression", tableName, fieldName);
        parseInfo.errDescr = i18n("More than one \"%1\" table or alias defined containing \"%2\" field.",
                                  tableName, fieldName);
        return false;
    }
    field = realField; //store
    tablePositionForField = tablePosition;
//    querySchema->addField(realField, tablePosition);

    return true;
}

//=========================================

static const char* const FunctionExpr_builtIns_[] = {"SUM", "MIN", "MAX", "AVG", "COUNT", "STD", "STDDEV", "VARIANCE", 0 };

class BuiltInAggregates : public QSet<QByteArray>
{
public:
    BuiltInAggregates() : QSet<QByteArray>() {
        for (const char * const *p = FunctionExpr_builtIns_; *p; p++)
            insert(QByteArray::fromRawData(*p, qstrlen(*p)));
    }
};

K_GLOBAL_STATIC(BuiltInAggregates, _builtInAggregates)

//=========================================

FunctionExpr::FunctionExpr(const QString& name_, NArgExpr* args_)
        : BaseExpr(0/*undefined*/)
        , name(name_.toUpper())
        , args(args_)
{
    if (_builtInAggregates->contains(name.toLatin1())) {
        m_cl = KexiDBExpr_Aggregation;
    }
    else {
        m_cl = KexiDBExpr_Function;
    }
    if (args)
        args->setParent(this);
}

FunctionExpr::FunctionExpr(const FunctionExpr& expr)
        : BaseExpr(0/*undefined*/)
        , name(expr.name)
        , args(expr.args ? args->copy() : 0)
{
    if (args)
        args->setParent(this);
}

FunctionExpr::~FunctionExpr()
{
    delete args;
}

FunctionExpr* FunctionExpr::copy() const
{
    return new FunctionExpr(*this);
}

QString FunctionExpr::debugString()
{
    QString res;
    res.append(QString("FunctionExpr(") + name);
    if (args)
        res.append(QString(",") + args->debugString());
    res.append(QString(",type=%1)").arg(Driver::defaultSQLTypeName(type())));
    return res;
}

QString FunctionExpr::toString(QuerySchemaParameterValueListIterator* params)
{
    return name + '(' + (args ? args->toString(params) : QString()) + ')';
}

void FunctionExpr::getQueryParameters(QuerySchemaParameterList& params)
{
    args->getQueryParameters(params);
}

Field::Type FunctionExpr::type()
{
    if (name == "SUBSTR") {
        if (args->containsNullArgument()) {
            return Field::Null;
        }
        return Field::Text;
    }
    //! @todo
    return Field::InvalidType;
}

bool FunctionExpr::validate(ParseInfo& parseInfo)
{
    if (!BaseExpr::validate(parseInfo)) {
        return false;
    }
    if (args->token() != ',') { // arguments required: NArgExpr with token ','
        return false;
    }
    if (!args->validate(parseInfo)) {
        return false;
    }
    if (name == "SUBSTR") {
        /* From https://www.sqlite.org/lang_corefunc.html:
        [1] substr(X,Y,Z)
        The substr(X,Y,Z) function returns a substring of input string X that begins
        with the Y-th character and which is Z characters long. If Z is omitted then

        [2] substr(X,Y)
        substr(X,Y) returns all characters through the end of the string X beginning with
        the Y-th. The left-most character of X is number 1. If Y is negative then the
        first character of the substring is found by counting from the right rather than
        the left. If Z is negative then the abs(Z) characters preceding the Y-th
        character are returned. If X is a string then characters indices refer to actual
        UTF-8 characters. If X is a BLOB then the indices refer to bytes. */
        if (args->containsInvalidArgument()) {
            return false;
        }
        const int count = args->args();
        if (count != 2 && count != 3) {
            parseInfo.errMsg = i18n("Incorrect number of arguments");
            parseInfo.errDescr = i18nc("@info Number of arguments error", "%1() function requires 2 or 3 arguments.", name);
            return false;
        }
        BaseExpr *textExpr = args->arg(0);
        if (!textExpr->isTextType() && textExpr->type() != Field::Null) {
            parseInfo.errMsg = i18n("Incorrect type of argument");
            parseInfo.errDescr = i18nc("@info Type error", "%1() function's first argument should be of type \"%2\". "
                                                           "Specified argument is of type \"%3\".",
                                       name,
                                       Field::typeName(Field::Text),
                                       Field::typeName(textExpr->type()));
            return false;
        }
        BaseExpr *startExpr = args->arg(1);
        if (!startExpr->isIntegerType() && startExpr->type() != Field::Null) {
            parseInfo.errMsg = i18n("Incorrect type of argument");
            parseInfo.errDescr = i18nc("@info Type error", "%1() function's second argument should be of type \"%2\". "
                                                           "Specified argument is of type \"%3\".",
                                       name,
                                       Field::typeName(Field::Integer),
                                       Field::typeName(startExpr->type()));
            return false;
        }
        BaseExpr *lengthExpr = 0;
        if (count == 3) {
            lengthExpr = args->arg(2);
            if (!lengthExpr->isIntegerType() && lengthExpr->type() != Field::Null) {
                parseInfo.errMsg = i18n("Incorrect type of argument");
                parseInfo.errDescr = i18nc("@info Type error", "%1() function's third argument should be of type \"%2\". "
                                                               "Specified argument is of type \"%3\".",
                                           name,
                                           Field::typeName(Field::Integer),
                                           Field::typeName(lengthExpr->type()));
                return false;
            }
        }
    }
    else {
        return false;
    }
    return true;
}

bool FunctionExpr::isBuiltInAggregate(const QByteArray& fname)
{
    return _builtInAggregates->contains(fname.toUpper());
}
