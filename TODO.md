# TODO: Connect C Backend to HTML Frontend

## Plan:
1. [ ] Update main.c - Add HTTP server code to handle requests and serve HTML
2. [ ] Update index.html - Remove all JavaScript, use HTML forms instead
3. [ ] Test the connected application

## Changes to main.c:
- Add HTTP server using sockets
- Parse HTTP requests
- Handle form submissions for each operation
- Serve HTML and return results

## Changes to index.html:
- Remove all <script> tags and JavaScript code
- Convert onclick buttons to HTML forms
- Use method="POST" or GET to communicate with C server
