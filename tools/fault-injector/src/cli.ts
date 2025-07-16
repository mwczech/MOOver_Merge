#!/usr/bin/env node

import { Command } from 'commander';
import inquirer from 'inquirer';
import chalk from 'chalk';
import axios from 'axios';
import { readFileSync, writeFileSync, existsSync } from 'fs';
import { join } from 'path';

interface FaultScenario {
  id: string;
  name: string;
  description: string;
  steps: FaultStep[];
  metadata: {
    category: 'safety' | 'performance' | 'environment' | 'communication';
    severity: 'low' | 'medium' | 'high' | 'critical';
    duration: number;
    repeatCount?: number;
  };
}

interface FaultStep {
  type: 'sensor' | 'motor' | 'environment' | 'system';
  action: string;
  target: string;
  value: any;
  delay: number;
  duration?: number;
}

const program = new Command();

// Default API endpoint
const DEFAULT_API_URL = 'http://localhost:3001';

// Pre-defined fault scenarios
const FAULT_SCENARIOS: FaultScenario[] = [
  {
    id: 'emergency_stop_test',
    name: 'Emergency Stop Response Test',
    description: 'Tests robot response to emergency stop signal',
    steps: [
      {
        type: 'system',
        action: 'inject_fault',
        target: 'emergency_system',
        value: { type: 'communication_error', component: 'emergency_system', severity: 'critical' },
        delay: 0
      }
    ],
    metadata: {
      category: 'safety',
      severity: 'critical',
      duration: 5000
    }
  },
  {
    id: 'sensor_failure_cascade',
    name: 'Sensor Failure Cascade',
    description: 'Simulates multiple sensor failures in sequence',
    steps: [
      {
        type: 'sensor',
        action: 'inject_sensor_value',
        target: 'magneticPosition',
        value: { x: 0, y: 0, detected: false },
        delay: 0,
        duration: 3000
      },
      {
        type: 'sensor',
        action: 'inject_sensor_value',
        target: 'battery',
        value: { voltage: 10.5, current: 0, percentage: 15 },
        delay: 1000,
        duration: 2000
      },
      {
        type: 'sensor',
        action: 'inject_sensor_value',
        target: 'temperature',
        value: [85, 87, 90],
        delay: 2000,
        duration: 1000
      }
    ],
    metadata: {
      category: 'safety',
      severity: 'high',
      duration: 5000
    }
  },
  {
    id: 'motor_block_scenario',
    name: 'Motor Blockage Simulation',
    description: 'Simulates motor being physically blocked',
    steps: [
      {
        type: 'motor',
        action: 'inject_motor_state',
        target: 'left_motor',
        value: { speed: 0, current: 2.5, temperature: 75, blocked: true },
        delay: 0,
        duration: 3000
      }
    ],
    metadata: {
      category: 'performance',
      severity: 'medium',
      duration: 3000
    }
  },
  {
    id: 'magnet_drift_test',
    name: 'Magnetic Field Drift',
    description: 'Simulates gradual magnetic field position drift',
    steps: [
      {
        type: 'environment',
        action: 'inject_magnet_shift',
        target: 'magnet_position',
        value: { fromPosition: { x: 100, y: 100 }, toPosition: { x: 105, y: 102 } },
        delay: 0
      },
      {
        type: 'environment',
        action: 'inject_magnet_shift',
        target: 'magnet_position',
        value: { fromPosition: { x: 105, y: 102 }, toPosition: { x: 110, y: 105 } },
        delay: 2000
      },
      {
        type: 'environment',
        action: 'inject_magnet_shift',
        target: 'magnet_position',
        value: { fromPosition: { x: 110, y: 105 }, toPosition: { x: 115, y: 108 } },
        delay: 4000
      }
    ],
    metadata: {
      category: 'environment',
      severity: 'medium',
      duration: 6000
    }
  },
  {
    id: 'power_fluctuation',
    name: 'Power Supply Fluctuation',
    description: 'Simulates unstable power supply conditions',
    steps: [
      {
        type: 'sensor',
        action: 'inject_sensor_value',
        target: 'battery',
        value: { voltage: 11.2, current: 1.8, percentage: 45 },
        delay: 0,
        duration: 1000
      },
      {
        type: 'sensor',
        action: 'inject_sensor_value',
        target: 'battery',
        value: { voltage: 10.8, current: 2.2, percentage: 42 },
        delay: 1000,
        duration: 1000
      },
      {
        type: 'sensor',
        action: 'inject_sensor_value',
        target: 'battery',
        value: { voltage: 11.5, current: 1.5, percentage: 47 },
        delay: 2000,
        duration: 1000
      }
    ],
    metadata: {
      category: 'performance',
      severity: 'medium',
      duration: 3000,
      repeatCount: 3
    }
  }
];

class FaultInjectorCLI {
  private apiUrl: string;

  constructor(apiUrl: string = DEFAULT_API_URL) {
    this.apiUrl = apiUrl;
  }

  async checkConnection(): Promise<boolean> {
    try {
      const response = await axios.get(`${this.apiUrl}/api/health`);
      return response.status === 200;
    } catch (error) {
      return false;
    }
  }

  async listScenarios(): Promise<void> {
    console.log(chalk.blue.bold('\n📋 Available Fault Scenarios:\n'));
    
    FAULT_SCENARIOS.forEach((scenario, index) => {
      const severityColor = scenario.metadata.severity === 'critical' ? 'red' :
                            scenario.metadata.severity === 'high' ? 'yellow' :
                            scenario.metadata.severity === 'medium' ? 'blue' : 'green';
      
      console.log(chalk.cyan(`${index + 1}. ${scenario.name}`));
      console.log(chalk.gray(`   ${scenario.description}`));
      console.log(chalk[severityColor](`   Severity: ${scenario.metadata.severity.toUpperCase()}`));
      console.log(chalk.gray(`   Category: ${scenario.metadata.category}`));
      console.log(chalk.gray(`   Duration: ${scenario.metadata.duration}ms`));
      console.log(chalk.gray(`   Steps: ${scenario.steps.length}\n`));
    });
  }

  async runScenario(scenarioId: string): Promise<void> {
    const scenario = FAULT_SCENARIOS.find(s => s.id === scenarioId);
    if (!scenario) {
      console.log(chalk.red(`❌ Scenario '${scenarioId}' not found`));
      return;
    }

    console.log(chalk.blue.bold(`\n🚀 Running scenario: ${scenario.name}\n`));
    console.log(chalk.gray(scenario.description));
    console.log(chalk.yellow(`⚠️  Severity: ${scenario.metadata.severity.toUpperCase()}\n`));

    const confirmed = await inquirer.prompt([
      {
        type: 'confirm',
        name: 'proceed',
        message: 'Do you want to proceed with this fault injection?',
        default: false
      }
    ]);

    if (!confirmed.proceed) {
      console.log(chalk.gray('Operation cancelled.'));
      return;
    }

    // Check API connection
    const connected = await this.checkConnection();
    if (!connected) {
      console.log(chalk.red(`❌ Cannot connect to API at ${this.apiUrl}`));
      console.log(chalk.gray('Make sure the simulator or hardware bridge is running.'));
      return;
    }

    console.log(chalk.green('✅ Connected to API'));
    console.log(chalk.blue('📡 Executing fault injection steps...\n'));

    // Execute scenario steps
    for (let i = 0; i < scenario.steps.length; i++) {
      const step = scenario.steps[i];
      
      if (step.delay > 0) {
        console.log(chalk.gray(`⏳ Waiting ${step.delay}ms...`));
        await this.sleep(step.delay);
      }

      console.log(chalk.cyan(`${i + 1}. ${step.action} -> ${step.target}`));
      
      try {
        await this.executeStep(step);
        console.log(chalk.green('   ✅ Success'));
      } catch (error) {
        console.log(chalk.red(`   ❌ Failed: ${error}`));
      }
    }

    console.log(chalk.green.bold('\n🎯 Scenario execution completed!'));
    
    if (scenario.metadata.repeatCount && scenario.metadata.repeatCount > 1) {
      const repeat = await inquirer.prompt([
        {
          type: 'confirm',
          name: 'repeat',
          message: `Repeat scenario ${scenario.metadata.repeatCount - 1} more times?`,
          default: false
        }
      ]);

      if (repeat.repeat) {
        for (let i = 1; i < scenario.metadata.repeatCount; i++) {
          console.log(chalk.blue(`\n🔄 Repetition ${i + 1}/${scenario.metadata.repeatCount}`));
          await this.sleep(1000);
          // Re-run steps (simplified)
          for (const step of scenario.steps) {
            if (step.delay > 0) await this.sleep(step.delay);
            try {
              await this.executeStep(step);
            } catch (error) {
              console.log(chalk.red(`Failed: ${error}`));
            }
          }
        }
      }
    }
  }

  async executeStep(step: FaultStep): Promise<void> {
    let endpoint = '';
    let payload = {};

    switch (step.action) {
      case 'inject_sensor_value':
        endpoint = '/api/events/inject-sensor';
        payload = {
          sensorType: step.target,
          value: step.value,
          duration: step.duration
        };
        break;
      
      case 'inject_motor_state':
        endpoint = '/api/events/inject-motor';
        payload = {
          motorId: step.target,
          state: step.value,
          duration: step.duration
        };
        break;
      
      case 'inject_magnet_shift':
        endpoint = '/api/events/inject-magnet-shift';
        payload = step.value;
        break;
      
      case 'inject_fault':
        endpoint = '/api/events/inject-fault';
        payload = step.value;
        break;
      
      default:
        throw new Error(`Unknown action: ${step.action}`);
    }

    const response = await axios.post(`${this.apiUrl}${endpoint}`, payload);
    if (!response.data.success) {
      throw new Error(response.data.error || 'API call failed');
    }
  }

  async interactiveMode(): Promise<void> {
    console.log(chalk.blue.bold('\n🎯 Interactive Fault Injection Mode\n'));

    const connected = await this.checkConnection();
    if (!connected) {
      console.log(chalk.red(`❌ Cannot connect to API at ${this.apiUrl}`));
      return;
    }

    while (true) {
      const action = await inquirer.prompt([
        {
          type: 'list',
          name: 'choice',
          message: 'What would you like to do?',
          choices: [
            { name: '📋 List available scenarios', value: 'list' },
            { name: '🚀 Run a scenario', value: 'run' },
            { name: '⚙️ Custom fault injection', value: 'custom' },
            { name: '📊 View active events', value: 'events' },
            { name: '🧹 Clear all events', value: 'clear' },
            { name: '🚪 Exit', value: 'exit' }
          ]
        }
      ]);

      switch (action.choice) {
        case 'list':
          await this.listScenarios();
          break;
        
        case 'run':
          const scenario = await inquirer.prompt([
            {
              type: 'list',
              name: 'id',
              message: 'Select scenario to run:',
              choices: FAULT_SCENARIOS.map(s => ({ name: s.name, value: s.id }))
            }
          ]);
          await this.runScenario(scenario.id);
          break;
        
        case 'custom':
          await this.customInjection();
          break;
        
        case 'events':
          await this.viewActiveEvents();
          break;
        
        case 'clear':
          await this.clearAllEvents();
          break;
        
        case 'exit':
          console.log(chalk.gray('Goodbye!'));
          return;
      }
    }
  }

  async customInjection(): Promise<void> {
    console.log(chalk.blue.bold('\n⚙️ Custom Fault Injection\n'));

    const injection = await inquirer.prompt([
      {
        type: 'list',
        name: 'type',
        message: 'Injection type:',
        choices: [
          { name: '📡 Sensor Value Override', value: 'sensor' },
          { name: '⚙️ Motor State Override', value: 'motor' },
          { name: '🌍 Environment Event', value: 'environment' },
          { name: '🚨 System Fault', value: 'fault' }
        ]
      }
    ]);

    switch (injection.type) {
      case 'sensor':
        await this.customSensorInjection();
        break;
      case 'motor':
        await this.customMotorInjection();
        break;
      case 'environment':
        await this.customEnvironmentInjection();
        break;
      case 'fault':
        await this.customFaultInjection();
        break;
    }
  }

  async customSensorInjection(): Promise<void> {
    const config = await inquirer.prompt([
      {
        type: 'list',
        name: 'sensor',
        message: 'Select sensor:',
        choices: [
          'magneticPosition',
          'imuData',
          'encoders',
          'battery',
          'temperature',
          'ultrasonicDistances'
        ]
      },
      {
        type: 'input',
        name: 'value',
        message: 'Enter value (JSON format):',
        default: '{"detected": false}'
      },
      {
        type: 'number',
        name: 'duration',
        message: 'Duration (ms, 0 = permanent):',
        default: 5000
      }
    ]);

    try {
      const value = JSON.parse(config.value);
      const step: FaultStep = {
        type: 'sensor',
        action: 'inject_sensor_value',
        target: config.sensor,
        value,
        delay: 0,
        duration: config.duration || undefined
      };

      await this.executeStep(step);
      console.log(chalk.green('✅ Sensor injection successful'));
    } catch (error) {
      console.log(chalk.red(`❌ Failed: ${error}`));
    }
  }

  async customMotorInjection(): Promise<void> {
    const config = await inquirer.prompt([
      {
        type: 'list',
        name: 'motor',
        message: 'Select motor:',
        choices: ['left_motor', 'right_motor', 'lift_motor']
      },
      {
        type: 'input',
        name: 'state',
        message: 'Enter motor state (JSON):',
        default: '{"speed": 0, "blocked": true}'
      },
      {
        type: 'number',
        name: 'duration',
        message: 'Duration (ms):',
        default: 3000
      }
    ]);

    try {
      const state = JSON.parse(config.state);
      const step: FaultStep = {
        type: 'motor',
        action: 'inject_motor_state',
        target: config.motor,
        value: state,
        delay: 0,
        duration: config.duration
      };

      await this.executeStep(step);
      console.log(chalk.green('✅ Motor injection successful'));
    } catch (error) {
      console.log(chalk.red(`❌ Failed: ${error}`));
    }
  }

  async customEnvironmentInjection(): Promise<void> {
    const config = await inquirer.prompt([
      {
        type: 'list',
        name: 'type',
        message: 'Environment event type:',
        choices: [
          'magnet_shift',
          'obstacle',
          'surface_change',
          'power_fluctuation'
        ]
      },
      {
        type: 'number',
        name: 'magnitude',
        message: 'Event magnitude (0-100):',
        default: 50
      }
    ]);

    const step: FaultStep = {
      type: 'environment',
      action: 'inject_environment_event',
      target: config.type,
      value: {
        type: config.type,
        magnitude: config.magnitude,
        position: { x: 100, y: 100 }
      },
      delay: 0
    };

    try {
      await this.executeStep(step);
      console.log(chalk.green('✅ Environment injection successful'));
    } catch (error) {
      console.log(chalk.red(`❌ Failed: ${error}`));
    }
  }

  async customFaultInjection(): Promise<void> {
    const config = await inquirer.prompt([
      {
        type: 'list',
        name: 'type',
        message: 'Fault type:',
        choices: [
          'motor_block',
          'sensor_failure',
          'power_loss',
          'communication_error'
        ]
      },
      {
        type: 'input',
        name: 'component',
        message: 'Component name:',
        default: 'system'
      },
      {
        type: 'list',
        name: 'severity',
        message: 'Severity:',
        choices: ['low', 'medium', 'high', 'critical']
      }
    ]);

    const step: FaultStep = {
      type: 'system',
      action: 'inject_fault',
      target: config.component,
      value: {
        type: config.type,
        component: config.component,
        severity: config.severity
      },
      delay: 0
    };

    try {
      await this.executeStep(step);
      console.log(chalk.green('✅ Fault injection successful'));
    } catch (error) {
      console.log(chalk.red(`❌ Failed: ${error}`));
    }
  }

  async viewActiveEvents(): Promise<void> {
    try {
      const response = await axios.get(`${this.apiUrl}/api/events/active`);
      const events = response.data.data;

      if (events.length === 0) {
        console.log(chalk.gray('\n📭 No active events\n'));
        return;
      }

      console.log(chalk.blue.bold(`\n📊 Active Events (${events.length}):\n`));
      
      events.forEach((event: any, index: number) => {
        console.log(chalk.cyan(`${index + 1}. ${event.description}`));
        console.log(chalk.gray(`   ID: ${event.id}`));
        console.log(chalk.gray(`   Type: ${event.type}`));
        console.log(chalk.gray(`   Target: ${event.target}`));
        console.log(chalk.gray(`   Duration: ${event.duration || 'permanent'}\n`));
      });
    } catch (error) {
      console.log(chalk.red(`❌ Failed to get events: ${error}`));
    }
  }

  async clearAllEvents(): Promise<void> {
    const confirmed = await inquirer.prompt([
      {
        type: 'confirm',
        name: 'clear',
        message: 'Clear all active events?',
        default: false
      }
    ]);

    if (!confirmed.clear) return;

    try {
      await axios.delete(`${this.apiUrl}/api/events/clear`);
      console.log(chalk.green('✅ All events cleared'));
    } catch (error) {
      console.log(chalk.red(`❌ Failed to clear events: ${error}`));
    }
  }

  private sleep(ms: number): Promise<void> {
    return new Promise(resolve => setTimeout(resolve, ms));
  }
}

// CLI Commands
program
  .name('fault-injector')
  .description('MELKENS Fault Injection Tool')
  .version('1.0.0');

program
  .option('-u, --url <url>', 'API endpoint URL', DEFAULT_API_URL)
  .option('-v, --verbose', 'Verbose output');

program
  .command('list')
  .description('List available fault scenarios')
  .action(async () => {
    const cli = new FaultInjectorCLI(program.opts().url);
    await cli.listScenarios();
  });

program
  .command('run <scenario>')
  .description('Run a specific fault scenario')
  .action(async (scenario) => {
    const cli = new FaultInjectorCLI(program.opts().url);
    await cli.runScenario(scenario);
  });

program
  .command('interactive')
  .alias('i')
  .description('Start interactive mode')
  .action(async () => {
    const cli = new FaultInjectorCLI(program.opts().url);
    await cli.interactiveMode();
  });

program
  .command('custom')
  .description('Custom fault injection')
  .option('-t, --type <type>', 'Injection type (sensor|motor|environment|fault)')
  .option('-T, --target <target>', 'Target component')
  .option('-v, --value <value>', 'Injection value (JSON)')
  .option('-d, --duration <duration>', 'Duration in milliseconds')
  .action(async (options) => {
    const cli = new FaultInjectorCLI(program.opts().url);
    
    if (!options.type || !options.target) {
      console.log(chalk.red('❌ Type and target are required for custom injection'));
      return;
    }

    try {
      const value = options.value ? JSON.parse(options.value) : {};
      const step: FaultStep = {
        type: options.type,
        action: `inject_${options.type === 'environment' ? 'environment_event' : options.type === 'system' ? 'fault' : options.type + '_value'}`,
        target: options.target,
        value,
        delay: 0,
        duration: options.duration ? parseInt(options.duration) : undefined
      };

      await cli.executeStep(step);
      console.log(chalk.green('✅ Custom injection successful'));
    } catch (error) {
      console.log(chalk.red(`❌ Failed: ${error}`));
    }
  });

program
  .command('scenarios')
  .description('Manage fault scenarios')
  .option('-e, --export <file>', 'Export scenarios to file')
  .option('-i, --import <file>', 'Import scenarios from file')
  .action(async (options) => {
    if (options.export) {
      try {
        writeFileSync(options.export, JSON.stringify(FAULT_SCENARIOS, null, 2));
        console.log(chalk.green(`✅ Scenarios exported to ${options.export}`));
      } catch (error) {
        console.log(chalk.red(`❌ Export failed: ${error}`));
      }
    }
    
    if (options.import) {
      if (!existsSync(options.import)) {
        console.log(chalk.red(`❌ File not found: ${options.import}`));
        return;
      }
      
      try {
        const data = readFileSync(options.import, 'utf8');
        const scenarios = JSON.parse(data);
        console.log(chalk.green(`✅ Loaded ${scenarios.length} scenarios from ${options.import}`));
        // In a real implementation, you'd merge or replace the scenarios
      } catch (error) {
        console.log(chalk.red(`❌ Import failed: ${error}`));
      }
    }
  });

// Default action - start interactive mode
program.action(async () => {
  console.log(chalk.blue.bold('🎯 MELKENS Fault Injector CLI'));
  console.log(chalk.gray('Use --help for available commands or start interactive mode\n'));
  
  const choice = await inquirer.prompt([
    {
      type: 'confirm',
      name: 'interactive',
      message: 'Start interactive mode?',
      default: true
    }
  ]);

  if (choice.interactive) {
    const cli = new FaultInjectorCLI(program.opts().url);
    await cli.interactiveMode();
  }
});

program.parse();