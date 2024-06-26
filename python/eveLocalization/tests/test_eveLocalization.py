# Copyright (c) CCP 2012

"""
Wrapper module to access blue exposed things from EVE's Localization DLL.
"""
from collections import OrderedDict
import sys
import unittest
import blue
import eveLocalization as el


def GetTestData():
    testData = {
        "en-us":
                {
                0: (u"This is a string with no tags.",
                    None,
                    None
                    ),
                1: (u"This is a {thing} with no parameters.",
                        {
                        'foo': u'bar',
                        },
                    OrderedDict([
                        (u"{thing}",
                             {
                             'conditionalValues': [],
                             'variableType': el.VARIABLE_TYPE.GENERIC,
                             'kwargs': {},
                             'propertyName': None,
                             'args': 0,
                             'variableName': 'thing'
                         }),
                    ])
                    ),
                2: (u"This string tests four types of modification of the latin alphabet: "
                    "{string1}, {string2}, {string3}, and {string4}.",
                    None,
                    OrderedDict([
                        (u"{string1}",
                             {
                             "conditionalValues": [],
                             "variableType": el.VARIABLE_TYPE.GENERIC,
                             "kwargs": {},
                             "propertyName": None,
                             "args": el.TOKEN_FLAG.CAPITALIZE,
                             "variableName": "string1"
                         }),
                        (u"{string2}",
                             {
                             "conditionalValues": [],
                             "variableType": el.VARIABLE_TYPE.GENERIC,
                             "kwargs": {},
                             "propertyName": None,
                             "args": el.TOKEN_FLAG.UPPERCASE,
                             "variableName": "string2"
                         }),
                        (u"{string3}",
                             {
                             "conditionalValues": [],
                             "variableType": el.VARIABLE_TYPE.GENERIC,
                             "kwargs": {},
                             "propertyName": None,
                             "args": el.TOKEN_FLAG.LOWERCASE,
                             "variableName": "string3"
                         }),
                        (u"{string4}",
                             {
                             "conditionalValues": [],
                             "variableType": el.VARIABLE_TYPE.GENERIC,
                             "kwargs": {},
                             "propertyName": None,
                             "args": el.TOKEN_FLAG.TITLECASE,
                             "variableName": "string4"
                         }),
                    ])
                    ),
                3: (u'You have {[numeric]numSeconds} {[numeric]numSeconds -> "second", "seconds"} to comply.',
                    None,
                    OrderedDict([
                        (u'{[numeric]numSeconds}',
                             {
                             'conditionalValues': [],
                             'variableType': el.VARIABLE_TYPE.NUMERIC,
                             'kwargs': {},
                             'propertyName': None,
                             'args': 0,
                             'variableName': 'numSeconds',
                             }),
                        (u'{[numeric]numSeconds -> "second", "seconds"}',
                             {
                             'conditionalValues': [u'second', u'seconds'],
                             'variableType': el.VARIABLE_TYPE.NUMERIC,
                             'kwargs': {},
                             'propertyName': None,
                             'args': el.TOKEN_FLAG.CONDITIONAL | el.TOKEN_FLAG.QUANTITY,
                             'variableName': 'numSeconds',
                             }),
                    ])
                    ),
                4: (u'This is a {thing} reusing a {thing} parameter.',
                    None,
                        {
                        u"{thing}":
                                {
                                'conditionalValues': [],
                                'variableType': el.VARIABLE_TYPE.GENERIC,
                                'kwargs': {},
                                'propertyName': None,
                                'args': 0,
                                'variableName': 'thing',
                                },
                        }
                    ),
                5: (u'Datetime formatting test: {[datetime]time, timeFormat=short}',
                    None,
                        {
                        u"{[datetime]time, timeFormat=short}":
                                {
                                'conditionalValues': [],
                                'variableType': el.VARIABLE_TYPE.DATETIME,
                                'kwargs': { "format": u"%Y.%m.%d %H:%M" },
                                'propertyName': None,
                                'args': 0,
                                'variableName': 'time',
                                }
                    }
                    ),
                6: (u'Datetime formatting test: {[datetime]time, timeFormat=long}',
                    None,
                        {
                        u"{[datetime]time, timeFormat=long}":
                                {
                                'conditionalValues': [],
                                'variableType': el.VARIABLE_TYPE.DATETIME,
                                'kwargs': { "format": u"%Y.%m.%d %H:%M:%S" },
                                'propertyName': None,
                                'args': 0,
                                'variableName': 'time',
                                }
                    }
                    ),
                7: (u'This is {[messageid]msg} within a message',
                    None,
                        {
                        u'{[messageid]msg}':
                                {
                                'conditionalValues': [],
                                'variableType': el.VARIABLE_TYPE.MESSAGE,
                                'kwargs': {},
                                'propertyName': None,
                                'args': 0,
                                'variableName': 'msg'
                            }
                    }
                    ),
                8: (u'a message',
                    None,
                    None
                    ),
                9: (u'Datetime formatting test: {[formattedtime]time, format="%A, %B %d %Y, %I:%M:%S %p"}',
                    None,
                        {
                        u'{[formattedtime]time, format="%A, %B %d %Y, %I:%M:%S %p"}':
                                {
                                'conditionalValues': [],
                                'variableType': el.VARIABLE_TYPE.FORMATTEDTIME,
                                'kwargs': { "format": u"%A, %B %d %Y, %I:%M:%S %p" },
                                'propertyName': None,
                                'args': 0,
                                'variableName': 'time',
                                }
                    }
                    ),
                10: (u'Numeric formatting test: {[numeric]number}',
                     None,
                         {
                         u'{[numeric]number}':
                                 {
                                 'conditionalValues': [],
                                 'variableType': el.VARIABLE_TYPE.NUMERIC,
                                 'kwargs': {},
                                 'propertyName': None,
                                 'args': 0,
                                 'variableName': 'number',
                                 },
                         }
                    ),
                11: (u'Numeric formatting test: {[numeric]number, decimalPlaces=0}',
                     None,
                         {
                         u'{[numeric]number, decimalPlaces=0}':
                                 {
                                 'conditionalValues': [],
                                 'variableType': el.VARIABLE_TYPE.NUMERIC,
                                 'kwargs': { "decimalPlaces": int(0), },
                                 'propertyName': None,
                                 'args': el.TOKEN_FLAG.DECIMALPLACES,
                                 'variableName': 'number',
                                 }
                     }
                    ),
                12: (u'Numeric formatting test: {[numeric]number, useGrouping}',
                     None,
                         {
                         u'{[numeric]number, useGrouping}':
                                 {
                                 'conditionalValues': [],
                                 'variableType': el.VARIABLE_TYPE.NUMERIC,
                                 'kwargs': {},
                                 'propertyName': None,
                                 'args': el.TOKEN_FLAG.USEGROUPING,
                                 'variableName': 'number',
                                 }
                     }
                    ),
                13: (u'Numeric formatting test: {[numeric]number, decimalPlaces=2}',
                     None,
                         {
                         u'{[numeric]number, decimalPlaces=2}':
                                 {
                                 'conditionalValues': [],
                                 'variableType': el.VARIABLE_TYPE.NUMERIC,
                                 'kwargs': { "decimalPlaces": 2 },
                                 'propertyName': None,
                                 'args': el.TOKEN_FLAG.DECIMALPLACES,
                                 'variableName': 'number'
                             }
                     }
                    ),
                14: (u'This is a linkinfo link: {[generic]linktext, linkinfo=linkdata}',
                     None,
                         {
                         u'{[generic]linktext, linkinfo=linkdata}':
                                 {
                                 'conditionalValues': [],
                                 'variableType': el.VARIABLE_TYPE.GENERIC,
                                 'kwargs': { "linkinfo": "linkdata" },
                                 'propertyName': None,
                                 'args': el.TOKEN_FLAG.LINKINFO,
                                 'variableName': 'linktext'
                             }
                     }
                    ),
                },
        "de":
                {
                1: (),
                },
        }
    return testData


testData = GetTestData()


class LocalizationUnittests(unittest.TestCase):
    """
    Make sure that the python exposure actually works as
    intended and prevent regressions from happening.
    """

    def setUp(self):
        el.LoadMessageData("en-us", testData["en-us"])

    def tearDown(self):
        el.UnloadMessageData("en-us")

    def testInvalidMessageDataTuple(self):
        """
        We don't know where this malformed data would come from, 
        but here is the defect: http://defects/issue.asp?ISID=76880
        """
        testData = {
            "ru":
                    {
                    0: (),
                    },
            }
        self.assertRaises(TypeError, el.LoadMessageData, 'ru', testData['ru'])

    def testCollator(self):
        """See if collator works."""
        col = el.Collator()
        col.locale = "en-us"
        self.assertEqual(col.locale, "en-us", "Collator has the wrong locale %s"%(col.locale,))
        res = col.Compare(u"a", u"b")
        self.assertEqual(res, -1, "a is not smaller than b")
        res = col.Compare(u"a", u"a")
        self.assertEqual(res, 0, "a is not a")
        res = col.Compare(u"b", u"a")
        self.assertEqual(res, 1, "b is not greater than a")

    def testLoadingData(self):
        """
        See if we can successfully store and retrieve data
        """
        data = el.GetMessageDataByID(1, "en-us")
        self.assertTrue(data == testData["en-us"][1], "message data storage does not return the same data for ID 1.")
        data = el.GetMessageDataByID(2, "en-us")
        self.assertTrue(data == testData["en-us"][2], "message data storage does not return the same data for ID 2.")


    def testParser(self):
        """
        Ensuring that our parser is providing all required functionality. Things that are not covered here rely upon
        undefined behaviour and thus may change and break in the future.
        """
        # no tags
        result = el.GetMessageByID(0, "en-us")
        expectedResult = u"This is a string with no tags."
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        
        # a basic tag ( with string parameter contents)
        result = el.GetMessageByID(1, "en-us", thing="basic tag")
        expectedResult = u"This is a basic tag with no parameters."
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        
        # a basic tag ( with numeric parameter contents)
        result = el.GetMessageByID(1, "en-us", thing=0)
        expectedResult = u"This is a 0 with no parameters."
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))

        # tag reuse
        result = el.GetMessageByID(4, "en-us", thing="basic tag")
        expectedResult = u"This is a basic tag reusing a basic tag parameter."
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        # testing generic formatting options
        #result = GetMessageByID(2, "en-us", string1="capitalization of strings",
        #                        string2="uppercase", string3="LOWERCASE", string4="title case")
        #expectedResult = u"This string tests four types of modification of the latin alphabet: " \
        #                  "Capitalization of strings, UPPERCASE, lowercase, and Title Case."
        #self.assertTrue(result == expectedResult,
        #                "Result did not match input: %s != %s" % (result, expectedResult))

        # formatted datetime, short (ID: 5) and long (ID: 6), both for all three cases (bluetime, tuple, float)
        tupleTimeVal = (2011, 2, 24, 11, 55, 00, 3, 55, 0)
        blueTimeVal  = blue.os.GetTimeFromParts(*tupleTimeVal[:7])
        floatTimeVal = 1.0 * 1298548500
        result = el.GetMessageByID(5, "en-us", time=tupleTimeVal)
        expectedResult = u"Datetime formatting test: 2011.02.24 11:55"
        self.assertTrue(result == expectedResult,
                        "Result did not match input %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(5, "en-us", time=blueTimeVal)
        expectedResult = u"Datetime formatting test: 2011.02.24 11:55"
        self.assertTrue(result == expectedResult,
                        "Result did not match input %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(5, "en-us", time=floatTimeVal)
        expectedResult = u"Datetime formatting test: 2011.02.24 11:55"
        self.assertTrue(result == expectedResult,
                        "Result did not match input %s != %s" % (result, expectedResult))

        result = el.GetMessageByID(6, "en-us", time=tupleTimeVal)
        expectedResult = u"Datetime formatting test: 2011.02.24 11:55:00"
        self.assertTrue(result == expectedResult,
                        "Result did not match input %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(6, "en-us", time=blueTimeVal)
        expectedResult = u"Datetime formatting test: 2011.02.24 11:55:00"
        self.assertTrue(result == expectedResult,
                        "Result did not match input %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(6, "en-us", time=floatTimeVal)
        expectedResult = u"Datetime formatting test: 2011.02.24 11:55:00"
        self.assertTrue(result == expectedResult,
                        "Result did not match input %s != %s" % (result, expectedResult))
        # formatted time - accepts a format string directly
        result = el.GetMessageByID(9, "en-us", time=tupleTimeVal)
        expectedResult = u"Datetime formatting test: Thursday, February 24 2011, 11:55:00 AM"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(9, "en-us", time=blueTimeVal)
        expectedResult = u"Datetime formatting test: Thursday, February 24 2011, 11:55:00 AM"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(9, "en-us", time=floatTimeVal)
        expectedResult = u"Datetime formatting test: Thursday, February 24 2011, 11:55:00 AM"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))

        # recursive messages
        result = el.GetMessageByID(7, "en-us", msg=8)
        expectedResult = u"This is a message within a message"
        self.assertTrue(result == expectedResult,
                        "Result did not match input %s != %s" % (result, expectedResult))

        # Numeric Quantities
        thousandSeparator = el.GetThousandSeparator("")
        decimalSeparator = el.GetDecimalSeparator("")
        result = el.GetMessageByID(10, "en-us", number=9999999)
        expectedResult = u"Numeric formatting test: 9999999"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(10, "en-us", number=9999999.12)
        expectedResult = f"Numeric formatting test: 9999999{decimalSeparator}12"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(11, "en-us", number=9999999.12)
        expectedResult = u"Numeric formatting test: 9999999"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(12, "en-us", number=9999999.123)
        expectedResult = f"Numeric formatting test: 9{thousandSeparator}999{thousandSeparator}999{decimalSeparator}12"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(13, "en-us", number=9999999.123)
        expectedResult = f"Numeric formatting test: 9999999{decimalSeparator}12"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(13, "en-us", number=9999999.1)
        expectedResult = f"Numeric formatting test: 9999999{decimalSeparator}10"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(13, "en-us", number=-9999999.1)
        expectedResult = f"Numeric formatting test: -9999999{decimalSeparator}10"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        # Numeric conditional statements
        result = el.GetMessageByID(3, "en-us", numSeconds=20)
        expectedResult = "You have 20 seconds to comply."
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(3, "en-us", numSeconds=1)
        expectedResult = u"You have 1 second to comply."
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.GetMessageByID(3, "en-us", numSeconds=1.7)
        expectedResult = f"You have 1{decimalSeparator}70 seconds to comply."
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))

        # Link creation mechanism ( With numeric arguments)
        result = el.GetMessageByID(14, "en-us", linktext="Click Here!", linkdata=("linkinfo", 100, 200, 300, 400, 500))
        expectedResult = u"This is a linkinfo link: <a href=linkinfo:100//200//300//400//500>Click Here!</a>"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        
        # Link creation mechanism ( With string arguments)
        result = el.GetMessageByID(14, "en-us", linktext="Click Here!", linkdata=("linkinfo", "100", "200", "300", "400", "500"))
        expectedResult = u"This is a linkinfo link: <a href=linkinfo:100//200//300//400//500>Click Here!</a>"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        
        # Link creation mechanism ( With numeric linkinfo argument)
        result = el.GetMessageByID(14, "en-us", linktext="Click Here!", linkdata=(0, "100", "200", "300", "400", "500"))
        expectedResult = u"This is a linkinfo link: <a href=0:100//200//300//400//500>Click Here!</a>"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        
        # Link creation mechanism ( With numeric linkinfo argument)
        result = el.GetMessageByID(14, "en-us", linktext=101.0123, linkdata=(0, "100", "200", "300", "400", "500"))
        expectedResult = u"This is a linkinfo link: <a href=0:100//200//300//400//500>101.0123</a>"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))


    def testNumericFormatter(self):
        """
        This is invoking the standalone method for number formatting
        """
        decimalSeparator = el.GetDecimalSeparator("")
        thousandSeparator = el.GetThousandSeparator("")
        result = el.FormatNumeric(999999, "en-us", useGrouping=False, decimalPlaces=0)
        expectedResult = "999999"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(999999, "en-us", useGrouping=True, decimalPlaces=0)
        expectedResult = f"999{thousandSeparator}999"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(999999.123, "en-us", useGrouping=True)
        expectedResult = f"999{thousandSeparator}999{decimalSeparator}12"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(999999.123, "en-us", useGrouping=True, decimalPlaces=1)
        expectedResult = f"999{thousandSeparator}999{decimalSeparator}1"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(999999.123, "en-us", useGrouping=True, decimalPlaces=0)
        expectedResult = f"999{thousandSeparator}999"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(-999999.123, "en-us", useGrouping=True, decimalPlaces=0)
        expectedResult = f"-999{thousandSeparator}999"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(-999999.123, "en-us", useGrouping=True, decimalPlaces=2)
        expectedResult = f"-999{thousandSeparator}999{decimalSeparator}12"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(0.0, "en-us")
        expectedResult = f"0{decimalSeparator}00" # decimalPlaces defaults to 2!
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(-9.123, "en-us", useGrouping=True, decimalPlaces=2, leadingZeroes=2)
        expectedResult = f"-09{decimalSeparator}12"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(-9000.123, "en-us", useGrouping=True, decimalPlaces=2, leadingZeroes=5)
        expectedResult = f"-09{thousandSeparator}000{decimalSeparator}12"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        result = el.FormatNumeric(-9.123, "en-us", useGrouping=True, decimalPlaces=2, leadingZeroes=5)
        expectedResult = f"-00{thousandSeparator}009{decimalSeparator}12"
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))


    def testWrapPoints(self):
        """
        Word wrapping tests...
        """
        a = el.WrapPointList(u"Hello World", "en-us")
        result = a.GetLinebreakPoints()
        expectedResult = [6,]
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        b = el.WrapPointList(u"The quick brown fox jumped over the lazy dog", "en-us")
        result = b.GetLinebreakPoints()
        expectedResult = [4, 10, 16, 20, 27, 32, 36, 41]
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))
        d = el.WrapPointList(u"1, 4,   5, 6. 1. 2. 3.", "en-us")
        result = d.GetLinebreakPoints()
        expectedResult = [3, 8, 11, 14, 17, 20]
        self.assertTrue(result == expectedResult,
                        "Result did not match input: %s != %s" % (result, expectedResult))

    def testWrapPointsForChinese(self):
        """
        According to Unicode, lines can break anywhere except before or after certain characters,
        and these may depend on user preference:
 
        "The second style of context analysis is used with East Asian ideographic and syllabic scripts.
         In these scripts, lines can break anywhere, except before or after certain characters.
        The precise set of prohibited line breaks may depend on user preference or local custom and is commonly tailorable."
            - https://www.unicode.org/reports/tr14/#Algorithm

        Since line breaks are determined by the OS APIs, we simply check that we get sane looking results.
        """
        CHINESE_STRING = "EVE姐妹会的主要职责是向那些遭遇战争、饥荒甚至迷失太空的人提供人道主义援助。然而，EVE姐妹济会的存在建立于与科学因素相关的强大宗教信仰之上。他们相信EVE之门是天堂的入口——神在门的另外一边。他们不光帮助那些需要帮助的人，还忙于进行关于EVE之门的科学试验，希望能够更深入地了解那里蕴涵的力量。"
        c = el.WrapPointList(CHINESE_STRING, "zh")
        result = c.GetLinebreakPoints()

        self.assertGreater(len(result), 0, "List should contain at least one wrap point")
        self.assertEqual(result, sorted(result), "Results should be sorted")
        self.assertEqual(result, sorted(list(set(result))), "Results should not contain duplicates")
        for i in result:
            self.assertIsInstance(i, int, "Line break points should be integers")

    def testWrapPointsForRTLLanguage(self):
        """
        Input is an RTL language (arabic) string. Should not contain word wrap points, especially not at position 0.
        """
        s = u"\u0633\u064a\u0645\u062a\u0634"
        d = el.WrapPointList(s, "en-us")
        self.assertFalse(d.GetLinebreakPoints())

    def testParseSentenceNoTokens(self):
        sentence = "The quick brown fox jumped over the lazy dog."
        result = el.Parse(sentence, "en-us", {})
        self.assertEqual(result, sentence)


if __name__ == '__main__':
    unittest.main()
