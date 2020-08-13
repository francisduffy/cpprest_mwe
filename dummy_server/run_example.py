"""Run payload n times."""

import logging
import argparse
import requests
import json
from distutils import util

if __name__ == '__main__':

    argparser = argparse.ArgumentParser(description='Run the payload n times.')
    argparser.add_argument('-l', '--loglevel', type=str, help='The log level.', default='INFO')
    argparser.add_argument('-p', '--server_uri', type=str, help='Server URI.', default='http://localhost:5003')
    argparser.add_argument('-n', '--num_runs', type=int, help='Number of runs.', default=1)
    argparser.add_argument('--docker', type=lambda x: bool(util.strtobool(x)),
                           help='Upload trades', default=False)
    args = argparser.parse_args()

    numeric_level = getattr(logging, args.loglevel.upper(), None)
    logging.basicConfig(format='%(asctime)s %(process)d [%(levelname)s] %(name)s: %(message)s', level=numeric_level)

    headers = {'Content-Type': 'application/json'}

    # Switch payload depending on whether server is in docker container or host.
    payload = {}
    if args.docker:
        payload['dataUri'] = 'http://host.docker.internal:8080/file/data.json'
    else:
        payload['dataUri'] = 'http://localhost:8080/file/data.json'

    # Run n times.
    for i in range(args.num_runs):

        try:
            logging.info('Send in request %d.', i)
            response = requests.post(args.server_uri, data=json.dumps(payload), headers=headers)
            response.raise_for_status()
            logging.info('Request returned with status code %d.', response.status_code)

        except requests.exceptions.HTTPError as e:
            logging.exception('There was a HTTP error running the request: %s.', str(e))

        except requests.exceptions.RequestException as e:
            logging.error('There was an exception running the request: %s.', str(e))
