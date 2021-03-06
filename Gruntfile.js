module.exports = function(grunt) {
	grunt.loadNpmTasks('grunt-shell');
	grunt.loadNpmTasks('grunt-contrib-watch');
	grunt.initConfig({
		shell: {
			greet: {
				command: function (greeting) {
					return 'echo ' + greeting;
				}
			},
			make: {
				command: 'make',
			},
			clean: {
				command: 'make clean',
			},
			execute: {
				// command: './worker 127.0.0.1 6379',
				command: './master 40000 60000',
			},
			test: {
				command: 'echo "TEST COMPLETE"',
			},
		},
		watch: {
			scripts: {
				files: ['**/*.c'],
					     tasks: ['run'],
					     options: {
					     spawn: false,
					     },
					     },
					     },
	});
	grunt.registerTask('default', ['watch']);
	grunt.registerTask('run', ['shell:clean', 'shell:make', 'shell:execute', 'shell:test']);
	grunt.registerTask('exec', ['shell:execute', 'shell:test']);
	grunt.registerTask('test', ['shell:test']);
}
