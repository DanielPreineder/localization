# Copyright © 2012 CCP ehf.

"""
You can run this from exefileInterpreter like this, assuming its batchfile is located in eve\client:
    exefileInterpreter.bat ../tools/testing/localization/benchmark.py

Alternately you can import this as a module somewhere (after adding this directory to the sitedir) and
simply call (attention: this unloads all message data):
    RunBenchmark(boot.role)

Make sure you have telemetry running if you want to get all the gory details. Alternately you can just
watch logserver to see overall performance values.

Feel free to add additional performance tests, the current set is a left-over from the time when we were
comparing MLS against Cerberus. There are no proper earlier records of performance values, though, so
you will want to keep track of them somewhere.
"""

# needed for our bloody annoying socket module modification, otherwise nasty won't import socketGPS.py
import socket
from eveprefs import prefs, boot


def foo(params):
    pass
socket.apply_settings = foo

import blue
import eveLocalization
import localization
import telemetry
import logging
import timeit

# keep me alive flag, otherwise we get killed after nasty is done bootstrapping
_running = True

logger = logging.getLogger()
logger.setLevel(logging.INFO)

# hook up logserver
import stdlogutils.logserver as stdlogsrv
stdlogsrv.InitLoggingToLogserver()


@telemetry.ZONE_FUNCTION
def WithoutMarkup(number=1000):
    """The most basic test. This is great to see performance of message retrievals.
    """
    t1 = timeit.Timer("""localization.GetByLabel("UI/Common/Unknown")""","import localization")
    logger.info("\t%s calls WithoutMarkup took %s seconds", number, t1.timeit(number=number))


@telemetry.ZONE_FUNCTION
def DateTimeFormatMarkup(number=1000):
    """Measure performance for what is probably the most complex operation in localization at the moment.
    """
    t1 = timeit.Timer("""localization.GetByLabel("UI/Inflight/Brackets/StructureProgress",
                         stateName=stateName, timeRemaining=long(timeLeft))""",
"""import const;import localization;
timeLeft = const.HOUR * 1 + const.MIN * 23 + const.SEC * 45 + const.MSEC * 67;
stateName = localization.GetByLabel("UI/Entities/States/Anchored")""")
    logger.info("%s calls to DateTimeFormatMarkup tooks %s seconds", number, t1.timeit(number=number))


@telemetry.ZONE_FUNCTION
def FormatDistInAU(number=1000):
    """Measure performance of a simple number formatting operation.
    """
    t1 = timeit.Timer("""FmtDist(dist, maxdemicals=1)""",
                      """from carbon.common.script.util.format import FmtDist;dist=const.AU*1.5""")
    logger.info("\t%s calls to FormatDistInAU Markup took %s seconds", number, t1.timeit(number=number))


def RunBenchmark(role="client"):
    # runs benchmarks for the given boot-role
    logger.info("Running %s localization benchmark", role)
    logger.info("\tUnloading message data...")
    eveLocalization.UnloadAllMessageData()
    boot.role = role
    logger.info("\tLoading message data...")
    localization.LoadLanguageData()
    WithoutMarkup()
    DateTimeFormatMarkup()
    # Disabled, crashes for some reason, probably an issue with not bootstrapping everything correctly
    #FormatDistInAU()


def Startup():
    """Bootstrap us if we run stand alone, and run client and proxy performance benchmarks

    NOTE: The localization module is currently (06/11/2012) doing some client optimizations on import.
          As such you need to change the code in locPackage.util.py that overrides GetLanguageID() to get
          absolutely perfect measurements. However, most of the time the performance overhead from that
          function is neglible and we really only want to measure message lookup and message parsing,
          which works awesomely with this.
    """
    # oh god, our codebase... prefs built-in is bootstrapped by importing inifile ...
    logger.info("Bootstrapping prefs...")
    import inifile
    inifile.Init()
    # we might need this commented out code below to properly initialize a session, GetLanguageID uses session.
    #logger.info("Bootstrapping eve namespace and session...")
    #import eve
    #import base # why?
    #import __builtin__
    #evetmp = eve.eve
    #evetmp.session = __builtin__.session
    #del eve
    #__builtin__.eve = evetmp
    logger.info("Starting telemetry...")
    blue.statistics.StartTelemetry("localhost")
    blue.synchro.Yield()
    RunBenchmark("proxy")
    RunBenchmark("client")
    global _running
    _running = False


if __name__ == "__main__":
    boot.role = "client"
    logger.info("Localization Benchmark Utility")
    t = blue.pyos.CreateTasklet(Startup, (), {})
    t.context = "^boot::localizationbenchmark"
    t.run()
    while _running:
        blue.os.Pump()
