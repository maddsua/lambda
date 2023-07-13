#include <string>

std::string pageTemplate = R"(
<!DOCTYPE html>
<html lang="en">
	<head>
		<meta charset="UTF-8">
		<meta http-equiv="X-UA-Compatible" content="IE=edge">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>${htmlpage_title}</title>
		<style>
			* {
				box-sizing: border-box;
				margin: 0;
				padding: 0;
			}
			body {
				display: flex;
				flex-direction: column;
				height: 100vh;
				align-items: center;
				justify-content: center;
				font-family: sans-serif;
				color: black;
				background-color: white;
			}
			.message {
				display: flex;
				flex-direction: row;
				gap: 1.25rem;
				align-items: center;
				justify-content: flex-start;
				flex-shrink: 0;
			}
			.status-code {
				font-weight: 400;
				font-size: 6rem;
			}
			.content {
				display: flex;
				flex-direction: column;
				gap: 0.625rem;
				align-items: flex-start;
				justify-content: flex-start;
				flex-shrink: 0;
				max-width: 22.5rem;
			}
			.message-title {
				font-weight: 400;
				font-size: 2.25rem;
			}
			.message-content {
				font-weight: 400;
				font-size: 1.125rem;
			}

		</style>
	</head>
	<body>
		<div class="message">
			<div class="status-code">${htmlpage_code}</div>
		
			<div class="content">
				<div class="message-title">${htmlpage_msg_title}</div>
				<div class="message-content">${htmlpage_msg_text}</div>
			</div>
		</div> 
	</body>
</html>
)";

