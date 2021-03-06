# -*- coding: utf-8 -*-
"""
 Copyright 2015 Telefonica Investigacion y Desarrollo, S.A.U

 This file is part of Orion Context Broker.

 Orion Context Broker is free software: you can redistribute it and/or
 modify it under the terms of the GNU Affero General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 Orion Context Broker is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero
 General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with Orion Context Broker. If not, see http://www.gnu.org/licenses/.

 For those usages not covered by this license please contact with
 iot_support at tid dot es
"""
__author__ = 'Iván Arias León (ivan dot ariasleon at telefonica dot com)'

import os

from termcolor import colored

from components.common_steps.initial_steps import *
from components.common_steps.requests import *

__logger__ = logging.getLogger("environment")


def __create_log_folder(name):
    """
    verify if the folder exists and it does not exists, it is created
    :param name: log folder name
    """
    try:
        if not os.path.exists(name):
            os.makedirs(name)
            __logger__.info("log folder has been created with name: %s" % name)
    except Exception, e:
        assert False, "ERROR  - creating logs folder \n       - %s" % str(e)


def before_all(context):
    """
    actions before all
    :param context: It’s a clever place where you and behave can store information to share around. It runs at three levels, automatically managed by behave.
    """
    __create_log_folder("logs")
    context.config.setup_logging(configfile="logging.ini")


def before_feature(context, feature):
    """
    actions before each feature
    in case of backgroundFeature, re-throw steps defined in the feature descriptions
    :param context: It’s a clever place where you and behave can store information to share around. It runs at three levels, automatically managed by behave.
    :param feature:
    """
    __logger__.info("\n\n\n\n")
    __logger__.info(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
    __logger__.info(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
    __logger__.info("BEFORE FEATURE: %s" % feature)
    __logger__.info("BEFORE FEATURE: %s" % feature.description)
    # ---- BackgroundFeature ----
    keywords = ["Setup:", "Check:"]
    for description in feature.description:
        if description.split(" ")[0] in keywords:
            description = description.replace(description.split(" ")[0], "Given")
            __logger__.debug("steps in Background Feature: %s" % description)
            context.execute_steps(description)


def after_feature(context, feature):
    """
    actions executed after each feature
    :param context: It’s a clever place where you and behave can store information to share around. It runs at three levels, automatically managed by behave.
    :param feature: feature properties
    """
    context.execute_steps(u'Given stop service')
    print colored('    And stop context Broker', 'green')
    __logger__.info("AFTER FEATURE")
    __logger__.info("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
    __logger__.info("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")


def before_scenario(context, scenario):
    """
    actions executed before each scenario
    :param context: It’s a clever place where you and behave can store information to share around. It runs at three levels, automatically managed by behave.
    :param scenario: scenario properties
    """
    __logger__.info("==>>")
    __logger__.info("BEFORE SCENARIO: %s " % scenario)


def after_scenario(context, scenario):
    """
    actions executed after each scenario
       - database used is deleted
    :param context: It’s a clever place where you and behave can store information to share around. It runs at three levels, automatically managed by behave.
    :param scenario: scenario properties
    """
    context.execute_steps(u'Given delete database in mongo')
    print colored('    And delete database in mongo', 'green')
    __logger__.info("AFTER SCENARIO")
    __logger__.info("<<==")


def before_step(context, step):
    """
   actions executed before each step
    :param context: It’s a clever place where you and behave can store information to share around. It runs at three levels, automatically managed by behave.
    :param step: step properties
    """
    __logger__.info("BEFORE STEP: %s" % step)


def after_step(context, step):
    """
   actions executed after each step
    :param context: It’s a clever place where you and behave can store information to share around. It runs at three levels, automatically managed by behave.
    :param step: step properties
    """
    __logger__.info("AFTER STEP")
