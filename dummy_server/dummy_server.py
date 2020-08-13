"""Very simple HTTP server in python."""

from http.server import BaseHTTPRequestHandler, HTTPServer
import threading
import logging
from setup_logging import setup_logging
import json
from lxml import etree
from lxml.html.builder import *
import os
import inspect
import argparse


class S(BaseHTTPRequestHandler):

    def _set_headers(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/ascii')
        self.end_headers()

    # noinspection PyPep8Naming
    def do_GET(self):

        logger = logging.getLogger(__name__)
        logger.info('Start handling GET request from %s.', self.path)

        # Split the path
        path_tokens = self.path.split('/')
        logger.info('Path tokens are %s.', path_tokens)

        # Send back different test responses depending on the path supplied.
        if path_tokens[1] == 'file' and len(path_tokens) == 3:

            filename = path_tokens[2]
            file_extension = filename.split('.')[-1]
            file_path = os.path.join(input_dir, filename)

            if file_extension == 'json':

                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()

                with open(file_path) as json_file:
                    body = json.load(json_file)

                self.wfile.write(json.dumps(body).encode())

            elif file_extension == 'xml':

                self.send_response(200)
                self.send_header('Content-Type', 'text/xml')
                self.end_headers()

                # Create an XML parser that removes white space
                parser = etree.XMLParser(remove_blank_text=True)

                # Load the XML file
                body = etree.parse(file_path, parser=parser)

                self.wfile.write(etree.tostring(body))

            else:

                self.send_response(500)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                body = {
                    'status_code': 500,
                    'message': 'Expected file to be either XML or JSON'
                }
                self.wfile.write(json.dumps(body).encode())

        else:
            self.send_response(200)

        logger.info('Finished handling GET request from %s.', self.path)

    # noinspection PyPep8Naming
    def do_HEAD(self):
        logger = logging.getLogger(__name__)
        logger.info('Handling HEAD request.')
        self._set_headers()

    # noinspection PyPep8Naming
    def do_POST(self):

        logger = logging.getLogger(__name__)
        logger.info('Start handling POST request from %s.', self.path)

        # Split the path
        path_tokens = self.path.split('/')
        logger.info('Path tokens are %s.', path_tokens)

        # Send back different test responses depending on the path supplied.
        if path_tokens[1] == 'result' and len(path_tokens) < 3:

            self.send_response(500)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            body = {
                'status_code': 500,
                'message': 'Need at least two path tokens in the /result path'
            }
            self.wfile.write(json.dumps(body).encode())

        elif path_tokens[1] == 'result':

            out_dir = output_dir
            for int_path in path_tokens[2:-1]:
                out_dir = os.path.join(out_dir, int_path)

            if not os.path.exists(out_dir):
                try:
                    os.makedirs(out_dir)
                    logger.debug('Created output directory %s', out_dir)
                except OSError as e:
                    logger.exception('Failed to create output directory %s: %s', out_dir, str(e))
                    raise

            filename = path_tokens[-1].replace('-', '_')
            file_extension = filename.split('.')[-1]
            store_path = os.path.join(out_dir, filename)
            self._set_headers()
            data = self.rfile.read(int(self.headers['Content-Length']))

            if file_extension != 'dat':
                with open(store_path, 'wb') as fh:
                    fh.write(data)

            self.send_response(200)

        elif path_tokens[1] == 'file' and len(path_tokens) == 3:

            # Print out the body of the POST request
            post_body = self.rfile.read(int(self.headers['Content-Length']))
            post_body = post_body.decode('utf8')
            logger.debug('Post body is:\n%s', post_body)

            filename = path_tokens[2]
            file_extension = filename.split('.')[-1]
            file_path = os.path.join(input_dir, filename)

            if file_extension == 'json':

                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()

                with open(file_path) as json_file:
                    body = json.load(json_file)

                self.wfile.write(json.dumps(body).encode())

            elif file_extension == 'xml':

                self.send_response(200)
                self.send_header('Content-Type', 'text/xml')
                self.end_headers()

                # Create an XML parser that removes white space
                parser = etree.XMLParser(remove_blank_text=True)

                # Load the XML file
                body = etree.parse(file_path, parser=parser)

                self.wfile.write(etree.tostring(body))

            else:

                self.send_response(500)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                body = {
                    'status_code': 500,
                    'message': 'Expected file to be either XML or JSON'
                }
                self.wfile.write(json.dumps(body).encode())

        else:
            self.send_response(200)

        logger.info('Finished handling POST request from %s.', self.path)

    def log_message(self, msg_format, *args):
        """This silences the console logging from the server.

        From here: https://stackoverflow.com/a/56230070/1771882
        """
        pass


class StatusHTTPServer(threading.Thread):

    def __init__(self, port=8080):
        self.logger = logging.getLogger(__name__)
        self.logger.info('Initialising the StatusHTTPServer on port %d', port)
        threading.Thread.__init__(self)
        self.running = False
        self.ready = False
        self.port = port
        self.httpd = None

    def run(self):
        self.logger.info('Running httpd...')
        self.running = True
        self.httpd.serve_forever()

    def start(self):
        self.logger.info('Starting httpd...')
        server_address = ('', self.port)
        HTTPServer.allow_reuse_address = True
        self.httpd = HTTPServer(server_address, S)
        threading.Thread.start(self)

    def stop(self):
        if self.httpd and self.running:
            self.logger.info('Shutting down httpd...')
            self.httpd.shutdown()
            self.httpd = None


if __name__ == '__main__':

    argparser = argparse.ArgumentParser(description='Simple dummy server')
    argparser.add_argument('--port', type=int, help='The server port', default=8080)
    argparser.add_argument('--input_dir', type=str, help='Path containing input files', default=None)
    argparser.add_argument('--output_dir', type=str, help='Path to write output', default=None)
    main_args = argparser.parse_args()

    setup_logging()
    main_logger = logging.getLogger(__name__)
    main_logger.info('Starting server script.')

    script_file_path = inspect.getfile(lambda: None)
    script_dir = os.path.dirname(script_file_path)
    main_logger.debug('Script directory is %s', script_dir)

    input_dir = main_args.input_dir
    if input_dir is None:
        input_dir = script_dir

    output_dir = main_args.output_dir
    if output_dir is None:
        output_dir = script_dir

    s = StatusHTTPServer(main_args.port)
    s.start()
