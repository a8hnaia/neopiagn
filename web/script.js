(async () => {
	const sourceElement = document.getElementById("source");
	const inputElement = document.getElementById("input");
	const outputElement = document.getElementById("output");
	const runElement = document.getElementById("run");
	let inputIndex = 0;

	const piagn = await Piagn({
		preRun: [ (piagn) => {
			const stdin = () => {
				if (inputIndex < inputElement.value.length) {
					inputIndex++;
					return inputElement.value.charCodeAt(inputIndex - 1);
				}
				else {
					return null;
				}
			};
			const stdout = (code) => {
				outputElement.value += String.fromCharCode(code);
			};
			const stderr = stdout;
			piagn.FS.init(stdin, stdout, stderr);
		}]
	});

	const initPiagn = piagn.cwrap("init_piagn", "number", []);
	const runPiagn = piagn.cwrap("run_piagn", null, ["number", "string"]);

	const piagnData = initPiagn();

	runElement.addEventListener("click", () => {
		outputElement.value = "";
		inputIndex = 0;
		console.log(sourceElement.value);
		runPiagn(piagnData, sourceElement.value);
	});
})();

