"""Function for setting up logging."""

import logging
import logging.config
import os
import json


def setup_logging(default_path='logging.json', default_level=logging.INFO, env_key='LOG_CFG'):
    """Set up logging configuration."""

    path = default_path
    value = os.getenv(env_key, None)

    if value:
        path = value

    if os.path.exists(path):

        with open(path, 'rt') as f:
            config = json.load(f)
        logging.config.dictConfig(config)

        logger = logging.getLogger(__name__)
        logger.info('Loaded logging configuration from %s.', path)

    else:
        logging.basicConfig(level=default_level)

        logger = logging.getLogger(__name__)
        logger.info('Using a basic logging configuration with level %s.', default_level)
